/*
 * test.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: bush
 */

#include "../../circuit_bmr/inc/BooleanCircuit.h"

#define test_circuit ("/home/bush/workspace/BMR/circuit_bmr/bin/test.txt")
#define aes_circuit ("/home/bush/workspace/BMR/circuit_bmr/bin/AES_example_test.txt")

#define aes_in1 ("0110")
#define aes_out1 ("01111010110101100111100101011011010000111111110100000100011111111101001011110001111101001101000110111111110010011000001011100011")

#define aes_in2 ("")
#define aes_out2 ("01100110111010010100101111010100111011111000101000101100001110111000100001001100111110100101100111001010001101000010101100101110")

int main () {

	std::string output;

	BooleanCircuit bc1(aes_circuit);
	bc1.RawInputs(aes_in1);
	bc1.Evaluate(1);
	output = bc1.Output();
	if(output != aes_out1) {
		perror("ERROR: not equal!!");
		exit(1);
	}

	BooleanCircuit bc2(aes_circuit);
	bc2.RawInputs(aes_in2);
	bc2.Evaluate(6);
	output = bc2.Output();
	if(output != aes_out2) {
		perror("ERROR: not equal!!");
		exit(1);
	}

	return 0;
}
