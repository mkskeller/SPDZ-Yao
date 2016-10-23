/*
 * common.h
 *
 *  Created on: Jan 27, 2016
 *      Author: bush
 */

#ifndef NETWORK_INC_COMMON_H_
#define NETWORK_INC_COMMON_H_

#include <string>
#include <stdexcept>

//#include "utils.h"

#define LENGTH_FIELD (4)

/*
 * To change the buffer sizes in the kernel
# echo 'net.core.wmem_max=12582912' >> /etc/sysctl.conf
# echo 'net.core.rmem_max=12582912' >> /etc/sysctl.conf
*/
const int BUFFER_SIZE = 20000000;

typedef struct {
	std::string ip;
	int port;
} endpoint_t;

typedef struct {
	const char* msg;
	unsigned int len;
} Msg;

#endif /* NETWORK_INC_COMMON_H_ */
