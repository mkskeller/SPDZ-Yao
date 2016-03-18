/*
 * Party.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: bush
 */

#include "Party.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <valgrind/callgrind.h>

#include "proto_utils.h"
#include "msg_types.h"
#include "prf.h"

#ifdef __PURE_SHE__
#include "mpirxx.h"
#endif

Party::Party(const char* netmap_file, // required to init Node
		   const char* circuit_file, // required to init BooleanCircuit
		   party_id_t id,
		   const std::string input,
		   int numthreads,
		   int numtries
		   ) :_id(id),
		   	  _all_input(input),
			  _NUMTHREADS(numthreads),
			  _NUMTRIES(numtries)
{
	_circuit = new BooleanCircuit( circuit_file );
	_G = _circuit->NumGates();
	_N = _circuit->NumParties();
	_W = _circuit->NumWires();
	_OW = _circuit->NumOutWires();
	_IO = _circuit->_num_input_wires;
	_circuit->_masks = new char[_W];
#ifdef __PURE_SHE__
	init_modulos();
#endif
	_initialize_input();
	_generate_prf_inputs();
	_allocate_prf_outputs();
	_allocate_input_wire_keys();
	_allocate_external_values();
	_num_externals_msg_received = {0};//ATOMIC_VAR_INIT(0);
	_num_inputkeys_msg_received = {0};//ATOMIC_VAR_INIT(0);
	printf("netmap_file: %s\n", netmap_file);
	if (0 == strcmp(netmap_file, LOOPBACK_STR)) {
		_node = new Node( NULL, id, this , _N+1);
	} else {
		_node = new Node( netmap_file, id, this );
	}
}

void Party::_allocate_external_values()
{
	_circuit->_externals = (char*)malloc(_W);
	memset(_circuit->_externals,NO_SIGNAL,_W);
}

void Party::_allocate_input_wire_keys ()
{
	_input_wire_keys_msg_sz = INPUT_KEYS_MSG_TYPE_SIZE+_IO*sizeof(Key);
	_input_wire_keys_msg = (char*)malloc(_input_wire_keys_msg_sz);
	memset(_input_wire_keys_msg, 0, _input_wire_keys_msg_sz);
}

void Party::_initialize_input()
{
	party_t me = _circuit->_parties[_id];
	std::string my_input = _all_input.substr(me.wires, me.n_wires);
	_input = new char[me.n_wires+1];
	const char* input = my_input.c_str();
	memcpy(_input, input, me.n_wires);

	for(int i=0; i<me.n_wires; i++) {
		_input[i]-=0x30;
	}
	printf("inputs:\n");
	phex(_input, me.n_wires);
}

Party::~Party() {
	// TODO Auto-generated destructor stub
}

void Party::NodeReady()
{
	printf("Node is ready\n");
}

void Party::NewMessage(int from, char* message, unsigned int len)
{
//	printf("got message of len %u from %d\n", len, from);
	MSG_TYPE message_type;
	memcpy(&message_type, message, sizeof(MSG_TYPE));
	switch(message_type) {
	case TYPE_KEYS:
		{
		printf("TYPE_KEYS\n");
		unsigned int keys_sz = 2*_W*_N*sizeof(Key);
		_circuit->_keys = (Key*)malloc(keys_sz);
		memcpy(_circuit->_keys, message+MSG_KEYS_HEADER_SZ, keys_sz);
//				phex(_circuit->Keys(), 2* _W * _N * sizeof(Key));
//				printf("\n");
//				_print_keys();

		_compute_prfs_outputs();
//				_print_prfs();
		_send_prfs();
//		printf("sent prfs\n");
		break;
		}
	case TYPE_MASK_INPUTS:
		{
		printf("TYPE_MASK_INPUTS\n");
		_generate_external_values_msg(message+sizeof(MSG_TYPE));
		break;
		}
	case TYPE_MASK_OUTPUT:
		{
		printf("TYPE_MASK_OUTPUT\n");
		memcpy(_circuit->Masks()+_circuit->OutWiresStart(), message+sizeof(MSG_TYPE), _OW);
		 	 	// TODO: only test!
//		 	 	 phex(_circuit->Masks(), _W);
		break;
		}
	case TYPE_GARBLED_CIRCUIT:
	{
		printf("TYPE_GARBLED_CIRCUIT\n");
		unsigned int garbled_tbl_sz = _G*4*_N*sizeof(Key);
		_circuit->_garbled_tbl = (Key*)malloc(garbled_tbl_sz);
		_circuit->_garbled_tbl_copy = (Key*)malloc(garbled_tbl_sz);
		memcpy(_circuit->_garbled_tbl, message+sizeof(MSG_TYPE), garbled_tbl_sz);
		memcpy(_circuit->_garbled_tbl_copy, message+sizeof(MSG_TYPE), garbled_tbl_sz);

//				printf("\nGarbled Table\n\n");
//				_printf_garbled_table();
		char garbled_circuit_cs = cs((char*)_circuit->_garbled_tbl , garbled_tbl_sz);
		printf ("\ngarbled_circuit_cs = %d\n", garbled_circuit_cs);

		char* received_gc_msg = new char[sizeof(MSG_TYPE)];
		fill_message_type(received_gc_msg, TYPE_RECEIVED_GC);
		_node->Send(SERVER_ID, received_gc_msg, sizeof(MSG_TYPE));

		break;
	}
	case TYPE_EXTERNAL_VALUES:
	{
		//this is done by party 1 only
//		printf("TYPE_EXTERNAL_VALUES from %d\n",from);
		_process_external_received(message+sizeof(MSG_TYPE), from);

		int num_received;
		{
		std::unique_lock<std::mutex> locker(_process_externals_mx);
		num_received = ++_num_externals_msg_received;
		}
		if(num_received == _N-1) {
			char* all_externals = new char[_IO+sizeof(MSG_TYPE)];
			fill_message_type(all_externals, TYPE_ALL_EXTERNAL_VALUES);
			memcpy(all_externals+sizeof(MSG_TYPE), _circuit->_externals,_IO);
			_node->Broadcast2(all_externals, _IO+sizeof(MSG_TYPE));
//			printf("sending all externals\n");
//			phex(all_externals+sizeof(MSG_TYPE), _IO);
		}
		break;
	}
	case TYPE_ALL_EXTERNAL_VALUES:
	{
//		printf("TYPE_ALL_EXTERNAL_VALUES\n");
//		phex(message+sizeof(MSG_TYPE), _IO);
		_process_all_external_received(message+sizeof(MSG_TYPE));
		fill_message_type(_input_wire_keys_msg, TYPE_KEY_PER_IN_WIRE);
		_node->Send(1, _input_wire_keys_msg, _input_wire_keys_msg_sz);
		break;
	}
	case TYPE_KEY_PER_IN_WIRE:
	{
//		printf("TYPE_KEY_PER_IN_WIRE from %d\n", from);
		_process_input_keys((Key*)(message+INPUT_KEYS_MSG_TYPE_SIZE), from);
		int num_received;
		{
			std::unique_lock<std::mutex> locker(_process_keys_mx);
			num_received = ++_num_inputkeys_msg_received;
		}
//		printf("num_received = %d\n",num_received);
		if(num_received == _N-1)
		{
//			printf("received input keys from everyone\n");
			unsigned int all_keys_msg_sz = 2*_N*_IO*sizeof(Key)+INPUT_KEYS_MSG_TYPE_SIZE;
			char* all_keys_msg = new char[all_keys_msg_sz];
			fill_message_type(all_keys_msg, TYPE_ALL_KEYS_PER_IN_WIRE);
			memcpy(all_keys_msg+INPUT_KEYS_MSG_TYPE_SIZE, _circuit->_keys, 2*_N*_IO*sizeof(Key));
//						printf("all input keys:\n");
//						_print_input_keys_checksum();
//						phex(all_keys_msg+INPUT_KEYS_MSG_TYPE_SIZE, 2*_N*_IO*sizeof(Key));
			_node->Broadcast2(all_keys_msg, all_keys_msg_sz);
			_check_evaluate();
		}


		break;
	}
	case TYPE_ALL_KEYS_PER_IN_WIRE:
	{
//					printf("TYPE_ALL_KEYS_PER_IN_WIRE\n");
		_process_all_input_keys(message+INPUT_KEYS_MSG_TYPE_SIZE);
					printf("all input keys:\n");
//					phex(message+INPUT_KEYS_MSG_TYPE_SIZE, 2*_N*_IO*sizeof(Key));
//					_print_input_keys_checksum();
		_check_evaluate();
		break;
	}
	case TYPE_LAUNCH_ONLINE:
	{
		printf("TYPE_LAUNCH_ONLINE\n");
		_start_online_net = GET_TIME();
//		_node->Broadcast2(_external_values_msg, _external_values_msg_sz);
		if(_id!=1) {
//					printf("sending my externals:\n");
//					phex(_external_values_msg+sizeof(MSG_TYPE), _circuit->Party(_id).n_wires);
			_node->Send(1,_external_values_msg, _external_values_msg_sz);
		}
		break;
	}
	case TYPE_CHECKSUM:
		{
		printf("TYPE_CHECKSUM\n");
		printf("got checksum = %d\n", message[sizeof(MSG_TYPE)]);
		break;
		}
	default:
		{
		printf("UNDEFINED\n");
		printf("got undefined message\n");
		phex(message, len);
		}
	}

}

void Party::_check_evaluate()
{
	_end_online_net = GET_TIME();
	printf("Network time: ");
	PRINT_DIFF(_start_online_net, _end_online_net);
#ifdef __PRIME_FIELD__
	printf("this implementation uses PRIME FIELD\n");
#endif

	printf("\n\npress for EVALUATING\n\n");
	getchar();

	exec_props_t* execs = new exec_props_t[_NUMTHREADS+1];
	unsigned long diff;
	for (int ntry=1; ntry<=_NUMTRIES; ntry++) {
		for(int nthreads=1; nthreads<=_NUMTHREADS; nthreads+=1) {
			memcpy(_circuit->_garbled_tbl, _circuit->_garbled_tbl_copy, _G*4*_N*sizeof(Key));
//				printf("num threads = %d\n", nthreads);

					CALLGRIND_START_INSTRUMENTATION;
			struct timeval* b;
			if(nthreads == 1) {
				b = GET_TIME();
//					printf("Linear evaluation\n");
				_circuit->EvaluateByLayerLinearly(_id);
			} else {
				b = GET_TIME();
//					printf("Non linear evaluation\n");
				_circuit->EvaluateByLayer(nthreads, _id);
			}
					CALLGRIND_STOP_INSTRUMENTATION;
					CALLGRIND_DUMP_STATS;
			struct timeval* a = GET_TIME();
//				_circuit->Output();

//				printf("Eval time: ");
			diff = GET_DIFF(b, a);
//			printf("#threads = %d, time = %lu\n", nthreads, diff);
			execs[nthreads].acc += diff;
			if (diff < execs[nthreads].min || execs[nthreads].min==0) {
				execs[nthreads].min = diff;
			}
		}
	}

	_circuit->Output();

	//getting the best results
	unsigned long minmin = execs[1].min;
	unsigned long long avgmin = execs[1].acc;
	int best_minmin  = 1;
	int best_avgmin  = 1;
	for(int nthreads=1; nthreads<=_NUMTHREADS; nthreads++) {
		printf("#nthreads = %d, min = %lu, acc = %llu\n",nthreads, execs[nthreads].min, execs[nthreads].acc);
		if(execs[nthreads].min < minmin) {
			minmin = execs[nthreads].min;
			best_minmin = nthreads;
		}
		if(execs[nthreads].acc < avgmin) {
			avgmin = execs[nthreads].acc;
			best_avgmin = nthreads;
		}
	}
	printf("\nRESULTS SUM:\n");
	printf("Got minimal time with %d threads: %ld\n", best_minmin, minmin);
	printf("Got minimal AVERAGE time with %d threads: %ld\n", best_avgmin, avgmin/_NUMTRIES);
}

void Party::Start()
{
	_node->Start();
}


void Party::_generate_prf_inputs()
{
	size_t size_of_inputs = _G*2*_N*16;
	char * prf_inputs = new char[size_of_inputs]; //G*2*n*128bit
	memset(prf_inputs, 0, size_of_inputs);

	/* fill out this buffer s.t. first 4 bytes are the extension (0/1),
	 * next 4 bytes are gate_id and next 4 bytes are party id.
	 * For the first half we dont need to fill the extension because
	 * it is zero anyway.
	 */
	unsigned int* prf_input_index = (unsigned int*)prf_inputs; //easier to refer as integers

	for(unsigned int e=0; e<=1; e++) {
		for (gate_id_t g=1; g<=_G; g++) {
			for(party_id_t j=1; j<=_N; j++) {
//				printf("e,g,j=%u,%u,%u\n",e,g,j);
				*prf_input_index = e;
				*(prf_input_index+1) = g;
				*(prf_input_index+2) = j;
				prf_input_index+=4;
			}
		}
	}
//	phex(prf_inputs, size_of_inputs);

	_circuit->PrfInputs(prf_inputs);
}

void Party::_allocate_prf_outputs() {
	unsigned int prf_output_size = (PRFS_PER_PARTY(_G, _N)*sizeof(Key) + RESERVE_FOR_MSG_TYPE ) *_N ;
	void* prf_outputs = malloc(prf_output_size);
	memset(prf_outputs,0,prf_output_size);
	_circuit->Prfs((char*)prf_outputs);

//	printf("prf_output_size = %u\n",prf_output_size);
//	printf("prf_outputs = %p-%p\n",prf_outputs, prf_outputs+prf_output_size);
}

void Party::_compute_prfs_outputs()
{
	unsigned int prf_outputs_offset = ( PRFS_PER_PARTY(_G,_N)*sizeof(Key) + RESERVE_FOR_MSG_TYPE )*(_id-1);
	char* party_prf_outputs_starting_point = _circuit->Prfs() + prf_outputs_offset;
	char* prf_outputs_index = party_prf_outputs_starting_point + RESERVE_FOR_MSG_TYPE;

//	printf("party_prf_outputs_starting_point = %p\n",party_prf_outputs_starting_point);

	for(gate_id_t g=1; g<=_G; g++) {
		wire_id_t in_wires[2] = {_circuit->_gates[g]._left, _circuit->_gates[g]._right};
		for(int w=0; w<=1; w++) {
			for (int b=0; b<=1; b++) {
				for (int e=0; e<=1; e++) {
					//TODO optimize by computing for all j's together
					for (int j=1; j<= _N; j++) {
						Key* key = _circuit->_key(_id, in_wires[w], b);
						char* input = _circuit->_input(e, g, j);
						PRF_single(key,input, prf_outputs_index);
#ifdef __PRIME_FIELD__
						((Key*)prf_outputs_index)->adjust();
#endif
						prf_outputs_index += RESERVE_FOR_MSG_TYPE;
					}
				}
			}
		}
	}
//	printf("\n\n");
//	phex(party_prf_outputs_starting_point, RESERVE_FOR_MSG_TYPE);
//	printf("\n");
//	phex(party_prf_outputs_starting_point+RESERVE_FOR_MSG_TYPE, PRFS_PER_PARTY(G,N)*sizeof(Key));
}

void Party::_send_prfs() {
	unsigned int prfs_size = PRFS_PER_PARTY(_G,_N)*sizeof(Key);
	unsigned int prf_outputs_offset = (prfs_size + RESERVE_FOR_MSG_TYPE) * (_id-1);

	char* party_prf_outputs_starting_point = _circuit->Prfs() + prf_outputs_offset + RESERVE_FOR_MSG_TYPE - sizeof(MSG_TYPE);
	fill_message_type(party_prf_outputs_starting_point, TYPE_PRF_OUTPUTS);

	_node->Send(SERVER_ID, party_prf_outputs_starting_point, prfs_size+sizeof(MSG_TYPE));
//	printf("\n");
//	phex(party_prf_outputs_starting_point, prfs_size+sizeof(MSG_TYPE));
}

void Party::_print_prfs()
{
	unsigned int prf_outputs_offset = ( PRFS_PER_PARTY(_G,_N)*sizeof(Key) + RESERVE_FOR_MSG_TYPE )*(_id-1);
	char* party_prf_outputs_starting_point = _circuit->Prfs() + prf_outputs_offset;
	char* prf_outputs_index = party_prf_outputs_starting_point + RESERVE_FOR_MSG_TYPE;

	for (gate_id_t g=1; g<=_G; g++) {
		wire_id_t in_wires[2] = {_circuit->_gates[g]._left, _circuit->_gates[g]._right};
		for(int w=0; w<=1; w++) {
			for (int b=0; b<=1; b++) {
				for (int e=0; e<=1; e++) {
//					phex(prf_outputs_index, _N*sizeof(Key));
//					printf("F_k^%d_{%u,%d}(%d,%u,1-n) = ", _id, in_wires[w], b, e, g);
					for(party_id_t j=1; j<=_N; j++) {
						Key k = *((Key*)(prf_outputs_index));
//						std::cout << k << "  ";
						prf_outputs_index += sizeof(Key);
					}
//					std::cout << std::endl<< std::endl;
				}
			}
		}
	}
}

void Party::_print_keys()
{
	for (wire_id_t w=0; w<_IO; w++) {
		for(int x=0; x<=1; x++) {
			printf("k^%d_{%u,%d}:  ",_id,w,x);
			Key* key_idx = _circuit->_key(_id,w,x);
			std::cout << *key_idx << std::endl;
		}
	}
}


void Party::_printf_garbled_table()
{
	for (gate_id_t g=1; g<=_G; g++) {
		std::cout << "gate " << g << std::endl;
		for(int entry=0; entry<4; entry++) {
			for(party_id_t i=1; i<=_N; i++) {
				Key* k = _circuit->_garbled_entry(g, entry);
				std::cout << *(k+i-1) << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}

void Party::_print_keys_of_party(Key *keys, int id)
{
	printf("\nkeys for party %d", id);
	for(wire_id_t w=0; w<_IO; w++) {
		printf("k^%d_%d: ", id, w);
		std::cout << keys[w] << std::endl;
	}
}

void Party::_print_input_keys_msg()
{
	for(wire_id_t w=0; w<_IO; w++) {
		std::cout << *(Key*)(_input_wire_keys_msg+INPUT_KEYS_MSG_TYPE_SIZE+w*sizeof(Key)) << std::endl;
	}
}




void Party::_generate_external_values_msg(char *masks)
{
	party_t me = _circuit->_parties[_id];
	wire_id_t s = me.wires; //the beginning of my wires
	wire_id_t n = me.n_wires; // number of my wires
	_external_values_msg_sz = sizeof(MSG_TYPE) + n;
	_external_values_msg = (char*)malloc (_external_values_msg_sz);
	fill_message_type(_external_values_msg, TYPE_EXTERNAL_VALUES);
	char *exv_msg = _external_values_msg+sizeof(MSG_TYPE);
	for(wire_id_t i=0; i<n; i++) {
		wire_id_t w = s+i;
		_circuit->_externals[w] = exv_msg[i] = masks[i]^_input[i];
		*(_circuit->_key(_id, w, 1-exv_msg[i])) = 0;
	}

//					phex(masks, n);
//					phex(_input, n);
//					phex(_external_values_msg+sizeof(MSG_TYPE), n);
}

void Party::_process_external_received(char* externals, party_id_t from)
{
	party_t sender = _circuit->_parties[from];
	wire_id_t s = sender.wires; //the beginning of my wires
	wire_id_t n = sender.n_wires; // number of my wires
	char* exv = _circuit->_externals;

//	printf("received externals from %d\n", from);
//	phex(externals, n);

	for(unsigned int i=0; i<n; i++) {
		wire_id_t w = s+i;
		exv[w] = externals[i];
		*(_circuit->_key(_id,w,1-exv[w])) = 0;
	}
}

void Party::_process_all_external_received(char* externals)
{
	memcpy(_circuit->_externals, externals, _IO);
	Key* keys_msg = (Key*)(_input_wire_keys_msg+INPUT_KEYS_MSG_TYPE_SIZE);
	for(wire_id_t w=0; w<_IO; w++) {
		keys_msg[w] = * _circuit->_key(_id,w,_circuit->_externals[w]);
	}

//	for(wire_id_t w=0; w<_IO; w++) {
//		printf("k^%d_{%u,%d}=", _id,w,_circuit->_externals[w]);
//		std::cout << keys_msg[w] << std::endl;
//	}
}

//void Party::_process_input_keys(char* keys, party_id_t from)
//{
////	printf("keys from party: %d\n",from);
//	for(wire_id_t w=0; w<_IO; w++) {
//		unsigned int offset = w*2*_N+_circuit->_externals[w]*_N+from-1;
//		_circuit->_keys[offset] = *(Key*)(keys+w*sizeof(Key));
////		printf("received key k^%d_{%u,%d}=", from,w,_circuit->_externals[w]);
////		std::cout <<_circuit->_keys[offset] << std::endl;
//	}
//}

void Party::_process_input_keys(Key* keys, party_id_t from)
{
//	printf("keys from party: %d\n",from);
//	for(wire_id_t w=0; w<_IO; w++) {
//		printf("k^%d_{%u,%d}=", from,w,_circuit->_externals[w]);
//		std::cout << keys[w] << std::endl;
//	}

	char *exv = _circuit->_externals;
	for(wire_id_t w=0; w<_IO; w++) {
		*(_circuit->_key(from,w,exv[w])) = keys[w];
	}
}


void Party::_process_all_input_keys(char* keys)
{
	/* keys: a block containing 2*_N keys for each input wire so the
	 * receiver only has to copy one time.
	*/
	memcpy(_circuit->_keys, keys, _IO*2*_N*sizeof(Key));
}

void Party::_print_input_keys_checksum()
{
	for (wire_id_t w=0; w<_IO; w++) {
		char x = _circuit->_externals[w];
		for(int x=0; x<=1; x++) {
			printf("k^I_{%u,%d}:  ",w,x);
			for (party_id_t i=1; i<=_N; i++) {
				Key* key_idx = _circuit->_key(i,w,x);
				std::cout << *key_idx << "  ";
			}
			std::cout << std::endl;
		}
	}
}


/* Gets as an argument a config file with the following format:
 * <netmap_file><newline>
 * <circuit_description_file><newline>
 * <party_id>
 */
int main(int argc, char *argv[]) {

	assert(argc==3);
	std::string config_file = argv[1];
	std::ifstream params(config_file);
	assert(params.good()); // manages to open the file
	party_id_t pid = atoi(argv[2]);

	int numthreads, numtrials;
	std::string netmap_path, circuit_path, input;
	params >> netmap_path >> circuit_path >> input >> numthreads >> numtrials;

#ifdef __PRIME_FIELD__
		printf("this implementation uses PRIME FIELD\n");
#endif

	Party* p = new Party(netmap_path.c_str(), circuit_path.c_str(), pid, input, numthreads, numtrials);
	p->Start();
	pause();

	return 0;
}
