/*
 * prf.h
 *
 *  Created on: Feb 18, 2016
 *      Author: bush
 */

#ifndef PROTOCOL_INC_PRF_H_
#define PROTOCOL_INC_PRF_H_

#include "Key.h"

void PRF_single(const Key* key, char* input, char* output);
void PRF_chunk(const Key* key, char* input, char* output, int number);

#endif /* PROTOCOL_INC_PRF_H_ */
