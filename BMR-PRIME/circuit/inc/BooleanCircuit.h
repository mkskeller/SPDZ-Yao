
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

#include "Wire.h"
#include "Gate.h"
#include "common.h"

typedef unsigned int party_id_t;


#define INIT_PARTY(W,N) {.wires=W, .n_wires=N }
typedef struct party_t{
	unsigned int wires; //the index of the first circuit input wire associated with this party
	unsigned int n_wires; //number of circuit input wires associated with this party
	void init(int w, int n){wires=w; n_wires=n;}
} party_t;



class BooleanCircuit
{
public:
	BooleanCircuit(const char* desc_file);
	void RawInputs(std::string raw_inputs);
	void Inputs(const char* inputs_file);
	void Evaluate(int num_threads);
	void EvaluateByLayerLinearly();
	virtual std::string Output();

	inline wire_id_t NumWires() { return _wires.size(); }

private:
	gate_id_t	_num_gates;
	party_id_t	 	_num_parties;
	int			_num_input_wires;
	int			_num_output_wires;
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


	std::set<gate_id_t> _ready_gates_set;
	std::atomic_int _num_evaluated_out_wires;
//	std::atomic_int _num_eval_gates;
	boost::mutex _ready_mx, _inc_mx;

	void _parse_circuit(const char* desc_file);
	void _eval_thread();
	virtual signal_t _eval_gate(gate_id_t g);

	inline bool is_wire_ready(wire_id_t w) {
		return  _wires[w].Sig()!=NO_SIGNAL;
	}
	inline bool is_gate_ready(gate_id_t g) {
		bool ready = is_wire_ready(_gates[g]._left)
						&& is_wire_ready(_gates[g]._right);
		return ready;
	}
};



#endif
