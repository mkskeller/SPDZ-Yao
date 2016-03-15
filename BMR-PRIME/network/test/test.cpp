

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include <boost/thread.hpp>

#include "test.h"
#include "utils.h"


char* buf;
#define buflen (5)


#define NMAP_FILE ("/home/bush/workspace/BMR/network/bin/nmap.txt")
#define SECOND (1000000)
#define MSECOND (1000)
#define BIG (30000000)

Test::Test(int id):_id(id), _stop(false) {
	srand (time(NULL));
	_node = new Node(NMAP_FILE, id, this);
	_node->Start();
}
void Test::NodeReady() {
	printf("\n\nTest:: Node Ready\n\n");
	/* sends various messages in random intervals */
	Msg msg = {0};
	_gen_message(msg);
	if(_id == 0) {
		for(int i=0; i<3; i++) {
	//		phex(msg.msg,msg.len);
			printf("Test:: broadcasting message of length %u\n", msg.len);
			_node->Broadcast(msg.msg,msg.len);
		}
	} else{
		for(int i=0; i<3; i++) {
			_node->Send(0, msg.msg, msg.len);
		}
	}
}
void Test::NewMessage(int from, char* msg, unsigned int len) {
	printf("\nTEST:: got message from %d of length %d\n",from,len);
//	phex(msg,len);
	char checksum = cs(msg, len-1);
	char lastbyte = msg[len-1];
	assert(checksum == lastbyte);

}

void Test::_gen_message(Msg& msg) {
	unsigned int size = rand()%(BIG/2)+BIG/2;
	char* buffer = new char[size+1];
	int nullfd = open("/dev/urandom", O_RDONLY);
	read(nullfd, buffer, size);
	close(nullfd);
	char checksum = cs(buffer,size);
	buffer[size] = checksum;

	msg.len = size+1;
	msg.msg = buffer;
}


int main (int argc, char** argv)
{
	if(argc < 2) {
		printf("MAIN:: 2nd argument missing\n");
		return 1;
	}

	int id = atoi(argv[1]);
	Test* t = new Test(id);

	pause();

//	while(true){
//		printf("MAIN:: sleeping for %u usecs\n",5*SECOND);
//		usleep(15*SECOND);
//	}

}
