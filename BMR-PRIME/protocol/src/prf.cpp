/*
 * prf.cpp
 *
 *  Created on: Feb 18, 2016
 *      Author: bush
 */


#include "prf.h"
#include "aes.h"
#include "proto_utils.h"

void PRF_single(const Key* key, char* input, char* output)
{
//				printf("prf_single\n");
//				std::cout << *key;
//				phex(input, 16);
	AES_KEY aes_key;
	AES_128_Key_Expansion((const unsigned char*)(&(key->r)), &aes_key);
	aes_key.rounds=10;
	AES_encryptC((block*)input, (block*)output, &aes_key);
//				phex(output, 16);
}

void PRF_chunk(const Key* key, char* input, char* output, int number)
{
	AES_KEY aes_key;
	AES_128_Key_Expansion((const unsigned char*)(&(key->r)), &aes_key);
	aes_key.rounds=10;
	AES_ecb_encrypt_chunk_in_out((block*)input, (block*)output, number, &aes_key);
}
