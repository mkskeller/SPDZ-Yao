// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * utils.h
 *
 */

#ifndef NETWORK_TEST_UTILS_H_
#define NETWORK_TEST_UTILS_H_


void fill_random(void* buffer, unsigned int length);

char cs(char* msg, unsigned int len, char result=0);

void phex (const void *addr, int len);

#endif /* NETWORK_TEST_UTILS_H_ */
