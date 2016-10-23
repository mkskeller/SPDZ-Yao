/*
 * utils.h
 *
 *  Created on: Jan 31, 2016
 *      Author: bush
 */

#ifndef PROTO_UTILS_H_
#define PROTO_UTILS_H_

#include "msg_types.h"
#include <time.h>
#include <sys/time.h>

#define LOOPBACK_STR "LOOPBACK"

void fill_random(void* buffer, unsigned int length);

void fill_message_type(void* buffer, MSG_TYPE type);

char cs(char* msg, unsigned int len, char result=0);

void phex (const void *addr, int len);

//inline void xor_big(const char* input1, const char* input2, char* output);


inline timeval* GET_TIME() {
	struct timeval* now = new struct timeval();
	int rc = gettimeofday(now, 0);
    if (rc != 0) {
        perror("gettimeofday");
    }
	return now;
}

inline unsigned long GET_DIFF(struct timeval* before, struct timeval* after) {
	long diff = ((after->tv_sec - before->tv_sec)*1000000L
	           +after->tv_usec) - before->tv_usec;
	return diff;
}

inline unsigned long PRINT_DIFF(struct timeval* before, struct timeval* after) {
	long diff = GET_DIFF(before, after);
	printf("Time in microseconds: %ld us\n", diff );
	return diff;
}



#endif /* NETWORK_TEST_UTILS_H_ */
