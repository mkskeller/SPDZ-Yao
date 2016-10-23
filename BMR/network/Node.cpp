/*
 * Node.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: bush
 */

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <boost/thread.hpp>
#include "utils.h"
#include "Node.h"

static void throw_bad_map_file() {
	fprintf(stderr,"Node:: ERROR: could not read map file\n");
	throw std::invalid_argument( "bad map file" );
}

static void throw_bad_id(int id) {
	fprintf(stderr,"Node:: ERROR: bad id %d\n",id);
	throw std::invalid_argument( "bad id" );
}

Node::Node(const char* netmap_file, int my_id, NodeUpdatable* updatable, int num_parties)
	:_id(my_id),
	 _updatable(updatable),
	 _connected_to_servers(false),
	 _num_parties_identified(0)
{
	_parse_map(netmap_file, num_parties);
	unsigned int max_message_size = BUFFER_SIZE/2;
	if(_id < 0 || _id > _numparties)
		throw_bad_id(_id);
	_ready_nodes = new bool[_numparties](); //initialized to false
	_clients_connected = new bool[_numparties]();
	_server = new Server(_port, _numparties-1, this, max_message_size);
	_client = new Client(_endpoints, _numparties-1, this, max_message_size);
}

Node::~Node() {
	delete(_client);
	delete(_server);
	delete (_endpoints);
	delete (_ready_nodes);
	delete (_clients_connected);
}


void Node::Start() {
	_client->Connect();
	new boost::thread(&Node::_start, this);
}

void Node::_start() {
	usleep(START_INTERVAL);
	while(true) {
		bool all_ready = true;
		if (_connected_to_servers) {
			for(int i=0; i<_numparties; i++) {
				if(i!=_id && _ready_nodes[i]==false) {
					all_ready = false;
					break;
				}
			}
		} else {
			all_ready = false;
		}
		if(all_ready)
			break;
		fprintf(stderr,"+");
//		fprintf(stderr,"Node:: waiting for all nodes to get ready ; sleeping for %u usecs\n", START_INTERVAL);
		usleep(START_INTERVAL);
	}
	printf("All identified\n");
	_client->Broadcast(ALL_IDENTIFIED,strlen(ALL_IDENTIFIED));
}

void Node::NewMsg(char* msg, unsigned int len, struct sockaddr_in* from) {
	//printf("Node:: got message of length %d ",len);
//	printf("from %s:%d\n", inet_ntoa(from->sin_addr), ntohs(from->sin_port));
	if(len == strlen(ID_HDR)+sizeof(_id) && 0==strncmp(msg, ID_HDR, strlen(ID_HDR))) {
		int *id = (int*)(msg+strlen(ID_HDR));
		printf("Node:: identified as party: %d\n", *id);
		assert(*id >=0 && *id <= _numparties && *id !=_id && !_clients_connected[*id]);
		_clients_connected[*id] = true;
		_clientsmap.insert(std::pair<struct sockaddr_in*,int>( from, *id));
		_ready_nodes[*id] = true;
		printf("Node:: _ready_nodes[%d]=%d\n",*id,_ready_nodes[*id]);
		return;
	} else if (len == strlen(ALL_IDENTIFIED) && 0==strncmp(msg, ALL_IDENTIFIED, strlen(ALL_IDENTIFIED))) {
		printf("Node:: received ALL_IDENTIFIED from %d\n",_clientsmap[from]);
		_num_parties_identified++;
		if(_num_parties_identified == _numparties-1) {
			printf("Node:: received ALL_IDENTIFIED from ALL\n",_clientsmap[from]);
			_updatable->NodeReady();
		}
		return;
	}
	_updatable->NewMessage(_clientsmap[from], msg, len );
}

void Node::ClientsConnected() {
	printf("Node:: Clients connected!\n");
}

void Node::NodeAborted(struct sockaddr_in* from)
{
	printf("Node:: party %d has aborted\n",_clientsmap[from]);
}

void Node::ConnectedToServers() {
	printf("Node:: Connected to all servers!\n");
	_connected_to_servers = true;
	_identify();
}

void Node::Send(int to, const char* msg, unsigned int len) {
	int new_recipient = to>_id?to-1:to;
	//printf("Node:: new_recipient=%d\n",new_recipient);
	_client->Send(new_recipient, msg, len);
}

void Node::Broadcast(const char* msg, unsigned int len) {
	_client->Broadcast(msg, len);
}
void Node::Broadcast2(const char* msg, unsigned int len) {
	_client->Broadcast2(msg, len);
}

void Node::_identify() {
	char* msg = new char[strlen(ID_HDR)+sizeof(_id)];
	strncpy(msg, ID_HDR, strlen(ID_HDR));
	strncpy(msg+strlen(ID_HDR), (const char *)&_id, sizeof(_id));
	//printf("Node:: identifying myself:\n");
	_client->Broadcast(msg,strlen(ID_HDR)+4);
}

void Node::_parse_map(const char* netmap_file, int num_parties) {
	if(LOOPBACK == netmap_file) {
		_numparties = num_parties;
		_endpoints = new endpoint_t[_numparties-1];
		int j=0;
		for(int i=0; i<_numparties; i++) {
			if(_id == i) {
				_ip = LOCALHOST_IP;
				_port = PORT_BASE + i;
				//printf("Node:: my address: %s:%d\n", _ip.c_str(),_port);
				continue;
			}
			_endpoints[j].ip = LOCALHOST_IP;
			_endpoints[j].port = PORT_BASE + i;
			j++;
		}
	}
	else {
		std::ifstream netmap(netmap_file);
		if(!netmap.good()) throw_bad_map_file();

		netmap >> _numparties;
		_endpoints = new endpoint_t[_numparties-1];
		int j=0;
		for(int i=0; i<_numparties; i++) {
			if(_id == i) {
				netmap >> _ip >> _port;
				//printf("Node:: my address: %s:%d\n", _ip.c_str(),_port);
				continue;
			}
			netmap >> _endpoints[j].ip >> _endpoints[j].port;
			j++;
		}
	}
}
