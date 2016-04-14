/*
 * test.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: bush
 */

#include "BooleanCircuit.h"

#define test_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/test.txt")
#define aes_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/AES_example_test.txt")
#define lt_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/lt.txt")
#define max3_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/max3.txt")
#define max6_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/max6.txt")

#define max3bm_circuit ("/home/bush/workspace/BMR-PRIME/circuit/bin/max3bm.txt")

#define aes_in1 ("0110")
#define aes_out1 ("01111010110101100111100101011011010000111111110100000100011111111101001011110001111101001101000110111111110010011000001011100011")

#define aes_in2 ("")
#define aes_out2 ("01100110111010010100101111010100111011111000101000101100001110111000100001001100111110100101100111001010001101000010101100101110")


#define N_LT_TESTS (4)
const char* lt_inputs[] = {"10001000011111110000000001111111",\
						"00000000011111111000100001111111",\
						"00001000000000010000100000000000",\
						"00001000000000000000100000000001"\
						};
const char* lt_outputs[] = {"1000100001111111",\
					 	 	"1000100001111111",\
							"0000100000000001",\
							"0000100000000001"\
						};

#define N_MAX3_TESTS (3)
const char* max3_inputs[] = {
							"100010000111111101001000011111111000100001111110",
							"000000000000000000000000000000000000000000000001",
							"000000000000000000000000000000010000000000000000"
							};
const char* max3_outputs[] = {
							"1000100001111111",
							"0000000000000001",
							"0000000000000001"
							};

#define N_MAX3BM_TESTS (3)
const char* max3bm_inputs[] = {
							"100010000111111101001000011111111000100001111110",
							"000000000000000000000000000000000000000000000001",
							"000000000000000000000000000000010000000000000000"
							};
const char* max3bm_outputs[] = {
							"1000100001111111100",
							"0000000000000001001",
							"0000000000000001010"
							};


#define N_MAX6_TESTS (3)
const char* max6_inputs[] = {
							"000100000000001001000000000001011000000000000001000000010000011100001000110001110000100001000111",
							"000100000000001000000001000001110000100011000111000010000100011100000000000000000100000000000101",
							"000000000000000000010000000000100000000100000111010000000000010100001000110001110000100001000111"
							};
const char* max6_outputs[] = {
							"1000000000000001",
							"0100000000000101",
							"0100000000000101"
							};

int main (int argc, char* argv[]) {

	std::string circuit_file = argv[1];
	std::string inputs = argv[2];

	std::string output;
	BooleanCircuit bc(circuit_file.c_str());
	bc.RawInputs(inputs);
	bc.EvaluateByLayerLinearly();
	output = bc.Output();



//	std::string output;
//	for (int i=0; i<N_LT_TESTS; i++) {
//		printf("\n");
//		BooleanCircuit bc(lt_circuit);
//		bc.RawInputs(lt_inputs[i]);
//		bc.EvaluateByLayerLinearly();
//		output = bc.Output();
//		if(0 != strcmp(output.c_str() ,lt_outputs[i])) {
//			perror("!!! ERROR: not equal!!");
//		}
//	}

//	for (int i=0; i<N_MAX3_TESTS; i++) {
//		printf("\n");
//		BooleanCircuit bc(max3_circuit);
//		bc.RawInputs(max3_inputs[i]);
//		bc.EvaluateByLayerLinearly();
//		output = bc.Output();
//		if(0 != strcmp(output.c_str() ,max3_outputs[i])) {
//			perror("!!! ERROR: not equal!!");
//		}
//	}

//	for (int i=0; i<N_MAX3BM_TESTS; i++) {
//		printf("\n");
//		BooleanCircuit bc(max3bm_circuit);
//		bc.RawInputs(max3bm_inputs[i]);
//		bc.EvaluateByLayerLinearly();
//		output = bc.Output();
//		if(0 != strcmp(output.c_str() ,max3bm_outputs[i])) {
//			perror("!!! ERROR: not equal!!");
//		}
//	}

//	for (int i=0; i<N_MAX6_TESTS; i++) {
//		printf("\n");
//		BooleanCircuit bc(max6_circuit);
//		bc.RawInputs(max6_inputs[i]);
//		bc.EvaluateByLayerLinearly();
//		output = bc.Output();
//		if(0 != strcmp(output.c_str() ,max6_outputs[i])) {
//			perror("!!! ERROR: not equal!!");
//		}
//	}

	return 0;
}
