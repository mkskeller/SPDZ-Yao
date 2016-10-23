/*
 * Server.h
 *
 *  Created on: Jan 24, 2016
 *      Author: bush
 */

#ifndef NETWORK_INC_SERVER_H_
#define NETWORK_INC_SERVER_H_

#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"

class ServerUpdatable {
public:
	virtual void ClientsConnected()=0;
	virtual void NewMsg(char* msg, unsigned int len, struct sockaddr_in* from)=0;
	virtual void NodeAborted(struct sockaddr_in* from) =0;
};

class Server {
public:
	Server(int port, int expected_clients, ServerUpdatable* updatable, unsigned int max_message_size);
	~Server();

private:
	int _expected_clients;
	int *_clients;
	struct sockaddr_in* _clients_addr;
	int _port;
	int _servfd;
	struct sockaddr_in _servaddr;

	ServerUpdatable* _updatable;
	unsigned int _max_msg_sz;

	void _start_server();
	void _listen_to_client(int id);
	bool _handle_recv_len(int id, unsigned int actual_len,unsigned int expected_len);
};



#endif /* NETWORK_INC_SERVER_H_ */
