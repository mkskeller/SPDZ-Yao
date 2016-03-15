/*
 * test.cpp
 *
 *  Created on: Feb 18, 2016
 *      Author: bush
 */

#include <iostream>
#include "Key.h"
#include "proto_utils.h"
#include "prf.h"

int main()
{
	Key a(0);
	Key b(1);
	a+=b;
	std::cout<< a << std::endl;

	char input[16]={0};
	char output[16]={0};

	phex(input, 16);
	phex(output, 16);

	for(int i=0; i<100; i++) {
		printf("\n");
		PRF_single((const Key*)&b, input, output);

	//	phex(input, 16);
		phex(output, 16);
	}

}



