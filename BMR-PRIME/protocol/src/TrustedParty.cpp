/*
 * TrustedParty.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: bush
 */

#include "TrustedParty.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>
#include <iostream>

#include "proto_utils.h"
#include "msg_types.h"




TrustedParty::TrustedParty(const char* netmap_file, // required to init Node
						   const char* circuit_file // required to init BooleanCircuit
						   )
{
	_circuit = new BooleanCircuit( circuit_file );
	_G = _circuit->NumGates();
	_N = _circuit->NumParties();
	_W = _circuit->NumWires();
	_OW = _circuit->NumOutWires();
	_allocate_prf_outputs();
	_allocate_garbled_table();
	_num_prf_received = 0;
	_received_gc_received = 0;
	if (0 == strcmp(netmap_file, LOOPBACK_STR)) {
		_node = new Node( NULL, 0, this , _N+1);
	} else {
		_node = new Node( netmap_file, 0, this );
	}
}

TrustedParty::~TrustedParty() {
	// TODO Auto-generated destructor stub
}

void TrustedParty::NodeReady()
{
	printf("\n\nNode ready \n\n");
	sleep(1);

	_generate_masks();

	unsigned int number_of_keys = 2* _W * _N;
	unsigned int size_of_keys = number_of_keys*sizeof(Key);
	unsigned int msg_keys_size = size_of_keys + MSG_KEYS_HEADER_SZ;

	Key* all_keys = new Key[number_of_keys];
	memset(all_keys, 0, size_of_keys);
	for(party_id_t pid=1; pid<=_N; pid++)
	{
		/* generating and sending keys */
		char* msg_keys = new char[msg_keys_size];
		memset(msg_keys, 0, msg_keys_size);
//						printf("keys for party %u\n", pid);
//						phex(msg_keys + sizeof(MSG_TYPE), size_of_keys);
		fill_message_type(msg_keys, TYPE_KEYS);
		_fill_keys_for_party((Key*)(msg_keys + MSG_KEYS_HEADER_SZ), pid);
//						printf("keys for party %u\n", pid);
//						phex(msg_keys + sizeof(MSG_TYPE), size_of_keys);
		_merge_keys(all_keys, (Key*)(msg_keys + MSG_KEYS_HEADER_SZ));

//				printf("all keys\n");
//				phex(all_keys, size_of_keys);
		_node->Send(pid, msg_keys, msg_keys_size);

		/* sending masks for input wires */
		party_t party = _circuit->Party(pid);
		char* msg_input_masks = new char[sizeof(MSG_TYPE) + party.n_wires];
		fill_message_type(msg_input_masks, TYPE_MASK_INPUTS);
		memcpy(msg_input_masks+sizeof(MSG_TYPE), _circuit->Masks()+party.wires, party.n_wires);
		_node->Send(pid, msg_input_masks, sizeof(MSG_TYPE) + party.n_wires);
				// TODO: test only
//				printf("input masks for party %d\n", pid);
//				phex(msg_input_masks + sizeof(MSG_TYPE) , party.n_wires);
	}

#ifdef __PRIME_FIELD__ //converting all keys to be from the prime field
	for(int i=0; i<number_of_keys; i++)
		all_keys[i].adjust();
#endif
	_circuit->Keys(all_keys);

	/* output wires' masks are the same for all players */
	/* sending masks for output wires */

	char* msg_output_masks = new char[sizeof(MSG_TYPE) + _OW];
	fill_message_type(msg_output_masks, TYPE_MASK_OUTPUT);
	memcpy(msg_output_masks + sizeof(MSG_TYPE),  _circuit->Masks()+_circuit->OutWiresStart(), _OW);
	_node->Broadcast(msg_output_masks, sizeof(MSG_TYPE) + _OW);
				// TODO: test only
//				printf("output masks\n");
//				phex(msg_output_masks+ sizeof(MSG_TYPE) , _OW);
}

void TrustedParty::NewMessage(int from, char* message, unsigned int len)
{

	MSG_TYPE message_type;
	memcpy(&message_type, message, sizeof(MSG_TYPE));
	switch(message_type) {
	case TYPE_PRF_OUTPUTS:
	{
		_print_mx.lock();
//		printf("got message of len %u from %d\n", len, from);
		printf("\n Got prfs from %d\n",from);

		char* party_prfs = _circuit->Prfs() + (PRFS_PER_PARTY(_G, _N)*sizeof(Key)) *(from-1) ;
		memcpy(party_prfs, message + sizeof(MSG_TYPE), PRFS_PER_PARTY(_G, _N)*sizeof(Key));
//		phex(party_prfs, PRFS_PER_PARTY(G, N)*sizeof(Key));
		_print_mx.unlock();

		_num_prf_received ++;
		if(_num_prf_received == _N) {
			_compute_send_garbled_circuit();
		}
		break;
	}
	case TYPE_RECEIVED_GC:
	{
		_received_gc_received++;
		if(_received_gc_received == _N) {
			_launch_online();
		}
		break;
	}
	default:
		{
		_print_mx.lock();
		printf("got message of len %u from %d\n", len, from);
		printf("UNDEFINED\n");
		printf("got undefined message\n");
		phex(message, len);
		_print_mx.unlock();
		}
	}

}

void TrustedParty::_launch_online()
{
	printf("press to launch online\n");
	getchar();
	char* launch_msg = new char[sizeof(MSG_TYPE)];
	fill_message_type(launch_msg, TYPE_LAUNCH_ONLINE);
	_node->Broadcast(launch_msg, sizeof(MSG_TYPE));
	printf("launched\n");
}

void TrustedParty::_allocate_garbled_table()
{
	unsigned int garbled_table_sz = _G*4*_N*sizeof(Key)+RESERVE_FOR_MSG_TYPE;
	_circuit->_garbled_tbl = (Key*)malloc(garbled_table_sz);
	memset(_circuit->_garbled_tbl,0,garbled_table_sz);

}

void TrustedParty::_compute_send_garbled_circuit()
{

	Key* tbl_start = _circuit->_garbled_tbl+(RESERVE_FOR_MSG_TYPE/sizeof(Key));
	unsigned int prfs_per_party_sz = PRFS_PER_PARTY(_G,_N)*sizeof(Key);
	std::vector<Gate>* gates = &_circuit->_gates;
	char* masks = _circuit->_masks;

	for(gate_id_t g=1; g<=_G; g++) {
//		std::cout << "garbling gate " << g << std::endl ;
		Key* gg_A = tbl_start + GARBLED_GATE_SIZE(_N)*(g-1);
		Key* gg_B = gg_A + _N;
		Key* gg_C = gg_A + 2*_N;
		Key* gg_D = gg_A + 3*_N;
		unsigned int prfs_left_offset = (g-1)*8*_N*sizeof(Key);
		unsigned int prfs_right_offset = prfs_left_offset + 4*_N*sizeof(Key);

		for(party_id_t i=1; i<=_N; i++) {
//			std::cout << "adding prfs of party " << i << std::endl ;
			char* party_prfs = _circuit->Prfs() + (i-1)*prfs_per_party_sz;
			Key left_i_j, right_i_j;
			for (party_id_t j=1; j<=_N; j++) {
				//A
//				std::cout << "A" << std::endl;
				left_i_j = *(Key*)(party_prfs+prfs_left_offset + (j-1)*sizeof(Key));
				right_i_j = *(Key*)(party_prfs+prfs_right_offset + (j-1)*sizeof(Key));
//							cout << *(gg_A+j-1) << std::endl;
//							cout << left_i_j << std::endl;
//							cout << right_i_j << std::endl;
				*(gg_A+j-1) += left_i_j; //left wire of party i in part j
				*(gg_A+j-1) += right_i_j; //right wire of party i in part j
//							cout << *(gg_A+j-1) << std::endl<< std::endl;

				//B
//				std::cout << "B" << std::endl;
				left_i_j = *(Key*)(party_prfs+prfs_left_offset + _N*sizeof(Key) + (j-1)*sizeof(Key));
				right_i_j = *(Key*)(party_prfs+prfs_right_offset + 2*_N*sizeof(Key) + (j-1)*sizeof(Key));
//							cout << *(gg_B+j-1) << std::endl;
//							cout << left_i_j << std::endl;
//							cout << right_i_j << std::endl;
				*(gg_B+j-1) += left_i_j; //left wire of party i in part j
				*(gg_B+j-1) += right_i_j; //right wire of party i in part j
//							cout << *(gg_B+j-1) << std::endl<< std::endl;

				//C
//				std::cout << "C" << std::endl;
				left_i_j = *(Key*)(party_prfs+prfs_left_offset + 2*_N*sizeof(Key) + (j-1)*sizeof(Key));
				right_i_j = *(Key*)(party_prfs+prfs_right_offset + _N*sizeof(Key) + (j-1)*sizeof(Key));
//							cout << *(gg_C+j-1) << std::endl;
//							cout << left_i_j << std::endl;
//							cout << right_i_j << std::endl;
				*(gg_C+j-1) += left_i_j; //left wire of party i in part j
				*(gg_C+j-1) += right_i_j; //right wire of party i in part j
//							cout << *(gg_C+j-1) << std::endl<< std::endl;

				//D
//				std::cout << "D" << std::endl;
				left_i_j = *(Key*)(party_prfs+prfs_left_offset + 3*_N*sizeof(Key) + (j-1)*sizeof(Key));
				right_i_j = *(Key*)(party_prfs+prfs_right_offset + 3*_N*sizeof(Key) + (j-1)*sizeof(Key));
//							cout << *(gg_D+j-1) << std::endl;
//							cout << left_i_j << std::endl;
//							cout << right_i_j << std::endl;
				*(gg_D+j-1) += left_i_j; //left wire of party i in part j
				*(gg_D+j-1) += right_i_j; //right wire of party i in part j
//							cout << *(gg_D+j-1) << std::endl<< std::endl;
			}
		}

		//Adding the hidden keys
		Gate* gate = &gates->at(g);
		char maskl = 	masks[gate->_left];
		char maskr = 	masks[gate->_right];
		char masko = 	masks[gate->_out];
//		printf("\ngate %u, leftwire=%u, rightwire=%u, outwire=%u: func=%d%d%d%d, msk_l=%d, msk_r=%d, msk_o=%d\n"
//				, g,gate->_left, gate->_right, gate->_out
//				,gate->_func[0],gate->_func[1],gate->_func[2],gate->_func[3], maskl, maskr, masko);

//		printf("\n");
//		printf("maskl=%d, maskr=%d, masko=%d\n",maskl,maskr,masko);
//		printf("gate func = %d%d%d%d\n",gate->_func[0],gate->_func[1],gate->_func[2],gate->_func[3]);
		bool xa = gate->_func[2*maskl+maskr] != masko;
		bool xb = gate->_func[2*maskl+(1-maskr)] != masko;
		bool xc = gate->_func[2*(1-maskl)+maskr] != masko;
		bool xd = gate->_func[2*(1-maskl)+(1-maskr)] != masko;
//		printf("xa=%d, xb=%d, xc=%d, xd=%d\n", xa,xb,xc,xd);

		// these are the 0-keys
		Key* outwire_start = _circuit->_keys + gate->_out*2*_N;
		Key* keyxa = outwire_start + (xa?_N:0);
		Key* keyxb = outwire_start + (xb?_N:0);
		Key* keyxc = outwire_start + (xc?_N:0);
		Key* keyxd = outwire_start + (xd?_N:0);

		for(party_id_t i=1; i<=_N; i++) {
//			std::cout << "adding to A = " << keyxa[i-1] << std::endl;
//			std::cout << "adding to B = " << keyxb[i-1] << std::endl;
//			std::cout << "adding to C = " << keyxc[i-1] << std::endl;
//			std::cout << "adding to D = " << keyxd[i-1] << std::endl;
			*(gg_A+i-1) += keyxa[i-1];
			*(gg_B+i-1) += keyxb[i-1];
			*(gg_C+i-1) += keyxc[i-1];
			*(gg_D+i-1) += keyxd[i-1];
		}
	}

	//sending to parties:
	fill_message_type(((char*)tbl_start)-4, TYPE_GARBLED_CIRCUIT );
	_node->Broadcast( ((char*)tbl_start)-4 , _G*4*_N*sizeof(Key)+sizeof(MSG_TYPE));

}

void TrustedParty::Start()
{
	_node->Start();
}

/* keys - a 2*W*n keys buffer to be filled only in the places belong to party pid */
void TrustedParty::_fill_keys_for_party(Key* keys, party_id_t pid)
{
	int nullfd = open("/dev/urandom", O_RDONLY);

	for (wire_id_t w=0; w<_W; w++) {
		read(nullfd, (char*)(keys+pid-1), sizeof(Key));
		read(nullfd, (char*)(keys+_N+pid-1), sizeof(Key));
		keys = keys + 2*_N;
	}
	close(nullfd);
}

/* Merge the two Key buffers into the dest buffer */
void TrustedParty::_merge_keys(Key* dest, Key* src)
{
	for (wire_id_t w=0; w<_W; w++) {
		for (int i=0; i<2; i++) {
			for (party_id_t p=0; p<_N; p++) {
				int offset = w*2*_N+i*_N+p;
				Key kk;
				kk = *(src + offset);
				dest[offset] += kk;
			}
		}
	}
}

void TrustedParty::_generate_masks	()
{
	char* masks = new char[_W];
	fill_random(masks, _W);

	//need to convert from total random to 0/1
	for (unsigned int i=0 ; i<_W; i++)
	{
		//because it is a SIGNED char ~half the chars whould be negetives
		masks[i] = masks[i]>0 ? 1 : 0;
	}
	_circuit->Masks(masks);
//	printf("masks\n");
//	phex(_circuit->Masks(), _W);
}


void TrustedParty::_allocate_prf_outputs()
{
	unsigned int prf_output_size = (PRFS_PER_PARTY(_G, _N)*sizeof(Key)) *_N ;
	void* prf_outputs = malloc(prf_output_size);
	memset(prf_outputs,0,prf_output_size);
	_circuit->Prfs((char*)prf_outputs);
}

void TrustedParty::_print_keys()
{
	Key* key_idx = _circuit->_key(1,0,0);
	for (wire_id_t w=0; w<_W; w++) {
		for(int b=0; b<=1; b++) {
			for (party_id_t i=1; i<=_N; i++) {
				printf("k^%d_{%u,%d}: ",i,w,b);
				std::cout << *key_idx << std::endl;
				key_idx++;
			}
		}
	}
}


/* Gets as an argument a config file with the following format:
 * <netmap_file><newline>
 * <circuit_description_file>
 */
int main(int argc, char *argv[]) {

	assert(argc==2);
	std::string config_file = argv[1];
	std::ifstream params(config_file);
	assert(params.good());

	std::string netmap_path, circuit_path;
	params >> netmap_path >> circuit_path;

#ifdef __PRIME_FIELD__
		printf("this implementation uses PRIME FIELD\n");
#endif

	TrustedParty* tp = new TrustedParty(netmap_path.c_str(), circuit_path.c_str());
	tp->Start();

	pause();

	return 0;
}
