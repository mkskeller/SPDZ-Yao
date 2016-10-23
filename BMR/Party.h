/*
 * Party.h
 *
 *  Created on: Feb 15, 2016
 *      Author: bush
 */

#ifndef PROTOCOL_PARTY_H_
#define PROTOCOL_PARTY_H_

#include "BooleanCircuit.h"
#include <mutex>
#include <boost/atomic.hpp>
#include "Node.h"

#define SERVER_ID (0)
#define INPUT_KEYS_MSG_TYPE_SIZE (16) // so memory will by alligned


typedef struct {
	unsigned long min=0;
	unsigned long long acc=0;
} exec_props_t;

class Party : public  NodeUpdatable {
public:
	Party(const char* netmap_file, const char* circuit_file, party_id_t id, const std::string input, int numthreads=5, int numtries=2);
	virtual ~Party();

	/* From NodeUpdatable class */
	void NodeReady();
	void NewMessage(int from, char* message, unsigned int len);
	void NodeAborted(struct sockaddr_in* from) {}

	void Start();

	/* TEST methods */

private:
	party_id_t _id;
	Node* _node;
	BooleanCircuit* _circuit;

	gate_id_t _G;
	party_id_t _N;
	wire_id_t _W;
	wire_id_t _OW;
	wire_id_t _IO;

	std::string _all_input;
	char* _input;

	char* _external_values_msg;
	unsigned int _external_values_msg_sz;
	int _num_externals_msg_received;
	std::mutex _process_externals_mx;

	char* _input_wire_keys_msg;
	unsigned int _input_wire_keys_msg_sz;
	int _num_inputkeys_msg_received;
	std::mutex _process_keys_mx;

	std::mutex _sync_mx;

//	int _num_evaluation_threads;
	struct timeval* _start_online_net, *_end_online_net;
	int _NUMTHREADS;
	int _NUMTRIES;

	void _initialize_input();
	void _generate_prf_inputs();
	void _allocate_prf_outputs();
	void _compute_prfs_outputs();
	void _print_prfs();
	void _send_prfs();
	void _print_keys();
	void _printf_garbled_table();

	void _allocate_external_values();
	void _generate_external_values_msg(char * masks);
	void _process_external_received(char* externals, party_id_t from);
	void _process_all_external_received(char* externals);
	inline void _allocate_input_wire_keys();
	void _print_input_keys_msg();
	void _print_keys_of_party(Key *keys, int id);
	void _print_input_keys_checksum();
	void _process_input_keys(Key* keys, party_id_t from);
	void _process_all_input_keys(char* keys);

	void _check_evaluate();

};

#endif /* PROTOCOL_PARTY_H_ */
