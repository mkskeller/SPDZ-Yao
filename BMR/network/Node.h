/*
 * Node.h
 *
 *  Created on: Jan 27, 2016
 *      Author: bush
 */

#ifndef NETWORK_NODE_H_
#define NETWORK_NODE_H_

#include <string>
#include <map>
#include <atomic>

#include "common.h"
#include "Client.h"
#include "Server.h"

class Server;
class Client;
class ServerUpdatable;
class ClientUpdatable;

class NodeUpdatable {
public:
	virtual void NodeReady()=0;
	virtual void NewMessage(int from, char* message, unsigned int len) =0;
	virtual void NodeAborted(struct sockaddr_in* from) =0;
};

typedef void (*msg_id_cb_t)(int from, char* msg, unsigned int len);
#define START_INTERVAL (3000000) // 5 secs
#define ID_HDR ("IDHD") // the chances to this to happen randomly is 2^(-64)
const char ALL_IDENTIFIED[] =  "ALID";
#define LOOPBACK NULL
#define LOCALHOST_IP "127.0.0.1"
#define PORT_BASE (4000)

class Node : public ServerUpdatable, public ClientUpdatable {
public:
	Node(const char* netmap_file, int my_id, NodeUpdatable* updatable, int num_parties=0);
	virtual ~Node();
	void Send(int to, const char* msg, unsigned int len);
	void Broadcast(const char* msg, unsigned int len);
	void Broadcast2(const char* msg, unsigned int len);
	inline int NumParties(){return _numparties;}
	void Start();
//	void Close();

	//derived from ServerUpdateable
	void NewMsg(char* msg, unsigned int len, struct sockaddr_in* from);
	void ClientsConnected();
	void NodeAborted(struct sockaddr_in* from);
	//derived from ClientUpdatable
	void ConnectedToServers();

private:
	void _parse_map(const char* netmap_file, int num_parties);
	void _identify();
	void _start();

	int _id;
	std::string _ip;
	int _port;
	int _numparties;

	endpoint_t* _endpoints;
	Client* _client;
	Server* _server;
	bool* _ready_nodes;
	volatile bool _connected_to_servers;
	std::atomic_int _num_parties_identified;

	std::map<struct sockaddr_in*,int> _clientsmap;
	bool* _clients_connected;
	NodeUpdatable* _updatable;
};

#endif /* NETWORK_NODE_H_ */
