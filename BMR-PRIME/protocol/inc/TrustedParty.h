/*
 * TrustedParty.h
 *
 *  Created on: Feb 15, 2016
 *      Author: bush
 */

#ifndef PROTOCOL_TRUSTEDPARTY_H_
#define PROTOCOL_TRUSTEDPARTY_H_


#include "BooleanCircuit.h"
#include "Node.h"
#include <atomic>


class TrustedParty : public  NodeUpdatable {

public:
	TrustedParty(const char* netmap_file,  const char* circuit_file );
	virtual ~TrustedParty();

	/* From NodeUpdatable class */
	void NodeReady();
	void NewMessage(int from, char* message, unsigned int len);
	void NodeAborted(struct sockaddr_in* from) {}

	void Start();


	/* TEST methods */

private:
	Node* _node;
	BooleanCircuit* _circuit;
	gate_id_t _G;
	party_id_t _N;
	wire_id_t _W;
	wire_id_t _OW;

	boost::mutex _print_mx;

	std::atomic_int _num_prf_received;
	std::atomic_int _received_gc_received;

	void _fill_keys_for_party(Key* keys, party_id_t pid);
	void _merge_keys(Key* dest, Key* src);
	void _generate_masks();
	void _allocate_prf_outputs();
	void _allocate_garbled_table();
	void _compute_send_garbled_circuit();
	void _launch_online();
	void _print_keys();

};

#endif /* PROTOCOL_TRUSTEDPARTY_H_ */
