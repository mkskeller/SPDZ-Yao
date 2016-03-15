/*
 * Client.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: bush
 */

#include "Client.h"
#include "common.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include <boost/thread.hpp>


static void throw_bad_ip(const char* ip) {
	fprintf(stderr,"Client:: Error: inet_aton - not a valid address? %s\n", ip);
	throw std::invalid_argument( "bad ip" );
}

Client::Client(endpoint_t* endpoints, int numservers, ClientUpdatable* updatable, unsigned int max_message_size)
	:_numservers(numservers),
	 _max_msg_sz(max_message_size),
	 _updatable(updatable),
	 _new_message(false)
	 {
	_sockets = new int[_numservers](); // 0 initialized
	_servers = new struct sockaddr_in[_numservers];
	_msg_queues = new Queue<Msg>[_numservers]();

	_lockqueue = new std::mutex[_numservers];
	_queuecheck = new std::condition_variable[_numservers];
	_new_message = new bool[_numservers]();

	memset(_servers, 0, sizeof(_servers));

	for (int i=0; i<_numservers; i++) {
		_sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if(-1 == _sockets[i])
			fprintf(stderr,"Client:: Error: socket: \n%s\n",strerror(errno));

		_servers[i].sin_family = AF_INET;
		_servers[i].sin_port = htons(endpoints[i].port);
		if(0 == inet_aton(endpoints[i].ip.c_str(), (in_addr*)&_servers[i].sin_addr))
			throw_bad_ip(endpoints[i].ip.c_str());
	}
}

Client::~Client() {
	for (int i=0; i<_numservers; i++)
		close(_sockets[i]);
	delete[] _sockets;
	delete[] _servers;
	delete[] _msg_queues;
	delete[] _lockqueue;
	delete[] _queuecheck;
	delete[] _new_message;
}

void Client::Connect() {
	for (int i=0; i<_numservers; i++)
		new boost::thread(&Client::_send_thread, this, i);
	new boost::thread(&Client::_connect, this);
}

void Client::_connect() {
	boost::thread_group tg;
	for(int i=0; i<_numservers; i++) {
		boost::thread* connector = new boost::thread(&Client::_connect_to_server, this, i);
		tg.add_thread(connector);
//		usleep(rand()%50000); // prevent too much collisions... TODO: remove
	}
	tg.join_all();
	_updatable->ConnectedToServers();
}

void Client::_connect_to_server(int i) {
	printf("Client:: connecting to server %d\n",i);
	char *ip;
	int port = ntohs(_servers[i].sin_port);
	ip = inet_ntoa(_servers[i].sin_addr);
	int error = 0;
	while (true ) {
		error = connect(_sockets[i], (struct sockaddr *)&_servers[i], sizeof(struct sockaddr));
		if(!error)
			break;
		if (errno == 111) {
			fprintf(stderr,".");
		} else {
			fprintf(stderr,"Client:: Error (%d): connect to %s:%d: \"%s\"\n",errno, ip,port,strerror(errno));
			fprintf(stderr,"Client:: socket %d sleeping for %u usecs\n",i, CONNECT_INTERVAL);
		}
		usleep(CONNECT_INTERVAL);
	}

	printf("\nClient:: connected to %s:%d\n", ip,port);
	setsockopt(_sockets[i], SOL_SOCKET, SO_SNDBUF, &BUFFER_SIZE, sizeof(BUFFER_SIZE));
}

void Client::Send(int id, const char* message, unsigned int len) {
	Msg new_msg = {message, len};
    {
        std::unique_lock<std::mutex> locker(_lockqueue[id]);
//        printf ("Client:: queued %u bytes to %d\n", len, id);
        _msg_queues[id].Enqueue(new_msg);
        _new_message[id] = true;
        _queuecheck[id].notify_one();
    }
}

void Client::Broadcast(const char* message, unsigned int len) {
	for(int i=0;i<_numservers; i++) {
		std::unique_lock<std::mutex> locker(_lockqueue[i]);
		Msg new_msg = {message, len};
        _msg_queues[i].Enqueue(new_msg);
        _new_message[i] = true;
        _queuecheck[i].notify_one();
	}
}

void Client::Broadcast2(const char* message, unsigned int len) {
	// first server is always the trusted party so we start with i=1
	for(int i=1;i<_numservers; i++) {
		std::unique_lock<std::mutex> locker(_lockqueue[i]);
		Msg new_msg = {message, len};
        _msg_queues[i].Enqueue(new_msg);
        _new_message[i] = true;
        _queuecheck[i].notify_one();
	}
}

void Client::_send_thread(int i) {
	while(true)
	{
		{
			std::unique_lock<std::mutex> locker(_lockqueue[i]);
			//printf("Client:: waiting for a notification to send to %d\n", i);
			_queuecheck[i].wait(locker);
			if (!_new_message[i]) {
//				printf("Client:: Spurious notification!\n");
				continue;
			}
			//printf("Client:: notified!!\n");
		}
		while (true)
		{
			Msg msg = {0};
			{
				std::unique_lock<std::mutex> locker(_lockqueue[i]);
				if(_msg_queues[i].Empty()) {
					//printf("Client:: no more messages in queue\n");
					break; // out of the inner while
				}
				msg = _msg_queues[i].Dequeue();
			}
			_send_blocking(msg, i);
		}
		_new_message[i] = false;
	}
}

void Client::_send_blocking(Msg msg, int id) {
//	printf ("Client:: sending %u bytes to %d\n", msg.len, id);
	int cur_sent = 0;
	cur_sent = send(_sockets[id], &msg.len, LENGTH_FIELD, 0);
	if(LENGTH_FIELD == cur_sent) {
		unsigned int total_sent = 0;
		unsigned int remaining = 0;
		while(total_sent != msg.len) {
			remaining = (msg.len-total_sent)>_max_msg_sz ? _max_msg_sz : (msg.len-total_sent);
			cur_sent = send(_sockets[id], msg.msg+total_sent, remaining, 0);
			//printf("Client:: msg.len=%u, remaining=%u, total_sent=%u, cur_sent = %d\n",msg.len, remaining, total_sent,cur_sent);
			if(cur_sent == -1) {
				fprintf(stderr,"Client:: Error: send msg failed: %s\n",strerror(errno));
				assert(cur_sent != -1);
			}
			total_sent += cur_sent;
		}
	} else if (-1 == cur_sent){
		fprintf(stderr,"Client:: Error: send header failed: %s\n",strerror(errno));
	}
}
