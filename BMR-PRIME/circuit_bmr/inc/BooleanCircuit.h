
#ifndef __BOOLEAN_CIRCUIT__
#define __BOOLEAN_CIRCUIT__

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <atomic>

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <mutex>
#include <condition_variable>

#include "proto_utils.h"
#include "common.h"
#include "Gate.h"
#include "Wire.h"
#include "Key.h"

typedef unsigned int party_id_t;

//#include "Party.h"


#define INIT_PARTY(W,N) {.wires=W, .n_wires=N }
typedef struct party_t{
	unsigned int wires; //the index of the first circuit input wire associated with this party
	unsigned int n_wires; //number of circuit input wires associated with this party
	void init(int w, int n){wires=w; n_wires=n;}
} party_t;


#define PRFS_PER_PARTY(G,n) (8*G*n)
#define RESERVE_FOR_MSG_TYPE (16) //Apparently must be a divided by 16 in order the aes encryption to work
#define GARBLED_GATE_SIZE(N) (4*N)
#define MSG_KEYS_HEADER_SZ (16)

//#define PAD_TO_8(n) (n+8-n%8)
#define PAD_TO_8(n) (n)

class BooleanCircuit
{
	friend class Party;
	friend class TrustedParty;
public:
	BooleanCircuit(const char* desc_file);
//	void RawInputs(std::string raw_inputs);
	void Inputs(const char* inputs_file);
	void Evaluate(int num_threads, party_id_t my_id);
	void EvaluateByLayer(int num_threads, party_id_t my_id);
	void EvaluateByLayerLinearly(party_id_t my_id);
	/*virtual*/ std::string Output();

	/* Usefull getters/setters */
	inline wire_id_t NumWires() { return _wires.size(); }
	inline wire_id_t NumOutWires() { return _num_output_wires; }
	inline wire_id_t OutWiresStart() { return _output_start; }
	inline gate_id_t NumGates() {return _num_gates; }
	inline wire_id_t NumParties() { return _num_parties; }
	inline party_t Party(party_id_t id) {return _parties[id];}
	inline void Keys(Key* keys) {_keys = keys;}
	inline Key* Keys() { return _keys; }
	inline void Masks(char* masks) {_masks = masks;}
	inline char* Masks() { return _masks; }
	inline char* PrfInputs() {return _prf_inputs;}
	inline void PrfInputs(char* prf_inputs) { _prf_inputs = prf_inputs; }
	inline char* Prfs(){return _prfs;}
	inline void Prfs(char* prfs){_prfs=prfs;}


private:
	gate_id_t	_num_gates;
	party_id_t	_num_parties;
	wire_id_t	_num_input_wires;
	wire_id_t	_num_output_wires;
	wire_id_t	_output_start; // output wires starts from that index

	std::vector<Gate> _gates; //begins with 1
	std::vector<Wire> _wires; //begins with 0
	std::vector<party_t> _parties; //begins with 1
	std::vector<gate_id_t> _input_gates; // ease the launch of the evaluation
	

	std::vector<std::vector<gate_id_t>> _layers;
	int _max_layer_sz;
	void _make_layers();
	int __make_layers(gate_id_t g);
	void _add_to_layer(int layer, gate_id_t g);
	void _print_layers();
	void _validate_layers();

	std::mutex _layers_mx;
	std::condition_variable _layers_cv;
	std::atomic_int _num_threads_finished;
	std::atomic_int _current_layer;
	std::atomic_int _evaluator_initiated;
	void _eval_by_layer(int i, int num_threads, party_id_t my_id);
	std::mutex _coordinator_mx;
	std::condition_variable _coordinator_cv;

	std::set<gate_id_t> _ready_gates_set;
	std::atomic_int _num_evaluated_out_wires;
	boost::mutex _ready_mx, _inc_mx;

	void _parse_circuit(const char* desc_file);
	void _eval_thread(party_id_t my_id);
//	/*virtual*/ signal_t _eval_gate(gate_id_t g, party_id_t my_id);
	/*virtual*/ signal_t _eval_gate(gate_id_t g, party_id_t my_id, char* prf_output);

	inline bool is_wire_ready(wire_id_t w) {
		return _externals[w] != NO_SIGNAL;
	}
	inline bool is_gate_ready(gate_id_t g) {
		bool ready = is_wire_ready(_gates[g]._left)
						&& is_wire_ready(_gates[g]._right);
		return ready;
	}

	/* Additional data stored per per party per wire: */
	Key* _keys; /* Total of n*W*2 keys
				 * 	For every w={0,...,W}
	 	 	 	 * 		For every b={0,1}
	 	 	 	 * 			For every i={1...n}
	 	 	 	 * 				k^i_{w,b}
	 	 	 	 * 	This is helpful that the keys for specific w and b are adjacent
	 	 	 	 * 	for pipelining matters.
	 	 	 	 */
	inline Key* _key(party_id_t i, wire_id_t w,int b) {return _keys+ w*2*_num_parties + b*_num_parties + i-1 ; }

	char* _masks; /* There are W masks, one per wire.  beginning with 0 */
	char* _externals; /* Same as _masks */

	Key* _garbled_tbl; /* will be allocated 4*n*G keys;
						* (n keys for each of A,B,C,D entries).
						* For each g in G:
						* 	A1, A2, ... , An
						* 	B1, B2, ... , Bn
						* 	C1, C2, ... , Cn
						* 	D1, D2, ... , Dn
						*/
	Key* _garbled_tbl_copy;
	inline Key* _garbled_entry(gate_id_t g, int entry) {return _garbled_tbl+(g-1)*4*_num_parties+entry*_num_parties;}

	char* _prfs; /*
				 *	Total of n*(G*2*2*2*n+4) = n*(8*G*n+RESERVE_FOR_MSG_TYPE) = n*(PRFS_PER_PARTY+RESERVE_FOR_MSG_TYPE)
				 *
				 *	For every party i={1...n}
				 *		<Message_type> = saves us copying memory to another location - only for Party.
				 *		For every gate g={1...G}
				 *			For input wires x={left,right}
				 *				For b={0,1} (0-key/1-key)
				 *					For e={0,1} (extension)
				 *						for every party j={1...n}
				 *							F_{k^i_{x,b}}(e,j,g,)
				 */


	char* _prf_inputs; /*
					   	   	   * These are all possible inputs to the prf,
					   	   	   * This is not efficient in terms of storage but increase
					   	   	   * performance in terms of speed since no need to generate
					   	   	   * (new allocation plus filling) those inputs every time
					   	   	   * we compute the prf.
					   	   	   * Structure:
					   	   	   * 	- Total of G*n*2 inputs. (i.e. for every gate g, for every
					   	   	   * 	party j and for every extension e (from {0,1}).
					   	   	   * 	- We want to be able to set key once and use it to encrypt
					   	   	   * 	several inputs, so we want those inputs to be adjacent in
					   	   	   * 	memory in order to save time of building the block of
					   	   	   * 	inputs. So, the structure of the inputs is as follows:
					   	   	   * 		- First half of inputs (G*n inputs) are:
					   	   	   * 			- For every gate g=1,...,G we store the inputs:
					   	   	   * 				(0||g||1),(0||g||2),...,(0||g||n)
					   	   	   * 		- Second half of the inputs are:
					   	   	   * 			- For every gate g=1,...,G we store the inputs:
					   	   	   * 				(1||g||1),(1||g||2),...,(1||g||n)
					   	   	   */
	inline char* _input(int e, gate_id_t g, party_id_t j)
	{ return _prf_inputs + (e*_num_gates*_num_parties + (g-1)*_num_parties + j-1) * 16 ;}

};



#endif
