
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <boost/thread.hpp>
#include <iostream>
#include <fstream>

#include "Server.h"


/* Opens server socket for listening - not yet accepting */
Server::Server(int port, int expected_clients, ServerUpdatable* updatable, unsigned int max_message_size)
	:_port(port),
	 _expected_clients(expected_clients),
	 _updatable(updatable),
	 _max_msg_sz(max_message_size)
{
	_clients = new int[expected_clients]();
	_clients_addr = new struct sockaddr_in[expected_clients]();

	_servfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == _servfd)
		printf("Server:: Error: socket: \n%s\n",strerror(errno));
	int set_option = 1;
	setsockopt(_servfd, SOL_SOCKET, SO_REUSEADDR, (char*)&set_option,sizeof(set_option));

	memset(&_servaddr, 0, sizeof(_servaddr));
	_servaddr.sin_family = AF_INET;
	_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_servaddr.sin_port   = htons(_port);

	if( 0 != bind(_servfd, (struct sockaddr *) &_servaddr, sizeof(_servaddr)) )
		printf("Server:: Error bind: \n%s\n",strerror(errno));

	if(0 != listen(_servfd, _expected_clients))
		printf("Server:: Error listen: \n%s\n",strerror(errno));


	new boost::thread(&Server::_start_server, this);
}

Server::~Server() {
	printf("Server:: Server being deleted\n");
	close(_servfd);
	for (int i=0; i<_expected_clients; i++)
		close(_clients[i]);
	delete (_clients);
	delete (_clients_addr);
}

void Server::_start_server() {
	socklen_t socksize = sizeof(struct sockaddr_in);
	printf("Server:: Ready for clients...\n");
	for(int i=0; i<_expected_clients; i++) {
        int fd = accept(_servfd, (sockaddr*)&_clients_addr[i], &socksize);
        _clients[i] = fd;
		printf("expected clients = %d, i=%d\n", _expected_clients, i);
        if (fd == -1) {
        	printf("Server:: accept: error in connecting socket\n%s\n",strerror(errno));
        } else {
			printf("Server:: Incoming connection from %s:%d\n",inet_ntoa(_clients_addr[i].sin_addr), ntohs(_clients_addr[i].sin_port));
			setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &BUFFER_SIZE, sizeof(BUFFER_SIZE));
			boost::thread* listener = new boost::thread(&Server::_listen_to_client, this, i);
        }
	}

	_updatable->ClientsConnected();
}

void Server::_listen_to_client(int id){
	int msg_len = 0;
	int n_recv = 0;
	unsigned int total_received;
	unsigned int remaining;
	char *msg;
	while (true) {
		n_recv = recv(_clients[id], &msg_len, LENGTH_FIELD, MSG_WAITALL);
		if (!_handle_recv_len(id, n_recv,LENGTH_FIELD))
			return;
//		printf("Server:: waiting for a message of len = %d\n", msg_len);
		msg = new char[msg_len];
		assert(msg != NULL);
		total_received = 0;
		remaining = 0;
		while (total_received != msg_len) {
			remaining = (msg_len-total_received)>_max_msg_sz ? _max_msg_sz : (msg_len-total_received);
			n_recv = recv(_clients[id], msg+total_received, remaining, NULL /* MSG_WAITALL*/);
//			printf("n_recv = %d\n", n_recv);
			if (!_handle_recv_len(id, n_recv,remaining)) {
				printf("returning\n");
				return;
			}
			total_received += n_recv;
//			printf("total_received = %d\n", total_received);
		}
//		printf("Server:: received %d: \n", msg_len);
		_updatable->NewMsg(msg, msg_len, &_clients_addr[id]);
	}
	printf("stop listenning to %d\n", id);
}

bool Server::_handle_recv_len(int id, unsigned int actual_len, unsigned int expected_len) {
//	printf("Server:: received msg from %d len = %u\n",id, actual_len);
	if (actual_len == 0) {
		printf("Server:: [%d]: Error: n_recv==0 Connection closed\n", id);
		_updatable->NodeAborted(&_clients_addr[id]);
		return false;
//		exit(1);
	} else if (actual_len == -1) {
		printf("Server:: [%d]: Error: n_recv==-1. \"%s\"\n",id, strerror(errno));
		_updatable->NodeAborted(&_clients_addr[id]);
		return false;
//		exit(1);
	} else if (actual_len < expected_len) {
//		printf("Server:: [%d]: Error: n_recv < %d; n_recv=%d; \"%d\"\n",id, expected_len, actual_len,strerror(errno));
		return true;
	}
}
