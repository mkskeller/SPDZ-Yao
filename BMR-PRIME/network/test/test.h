/*
 * test.h
 *
 *  Created on: Jan 30, 2016
 *      Author: bush
 */

#ifndef NETWORK_TEST_TEST_H_
#define NETWORK_TEST_TEST_H_

#include "Node.h"


class Test : public  NodeUpdatable {
public:
	Test(int id);
	void NodeReady();
	void NewMessage(int from, char* message, unsigned int len);
	void NodeAborted(struct sockaddr_in* from) {}
	void Stop(){_stop=true;}
private:
	void _gen_message(Msg& msg );

	int _id;
	Node* _node;
	bool _stop;
};


#endif /* NETWORK_TEST_TEST_H_ */
