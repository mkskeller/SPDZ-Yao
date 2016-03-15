/*
 * Client.h
 *
 *  Created on: Jan 27, 2016
 *      Author: bush
 */

#ifndef NETWORK_INC_CLIENT_H_
#define NETWORK_INC_CLIENT_H_

#include "common.h"
#include "mq.h"
//#include <queue>
//#include <boost/thread.hpp>
//#include <boost/thread/mutex.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

#define CONNECT_INTERVAL (1000000)

class ClientUpdatable {
public:
	virtual void ConnectedToServers()=0;
};




class Client {
public:
	Client(endpoint_t* servers, int numservers, ClientUpdatable* updatable, unsigned int max_message_size);
	virtual ~Client();

	void Connect();
	void Send(int id, const char* message, unsigned int len);
	void Broadcast(const char* message, unsigned int len);
	void Broadcast2(const char* message, unsigned int len);

private:

	Queue<Msg>* _msg_queues;
//	std::queue<Msg>* _msg_queues;
//	boost::mutex* _msg_mux;
//	boost::mutex* _thd_mux;
	unsigned int _max_msg_sz;
	std::mutex*              _lockqueue;
	std::condition_variable* _queuecheck;
	bool*					 _new_message;
	void _send_thread(int i);
	void _send_blocking(Msg msg, int id);

	void _connect();
	void _connect_to_server(int i);

	int _numservers;
	struct sockaddr_in* _servers;
	int* _sockets;
	ClientUpdatable* _updatable;
};

#endif /* NETWORK_INC_CLIENT_H_ */
