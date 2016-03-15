/*
 * utils.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: bush
 */



#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
//#include <intrin.h>

#include "proto_utils.h"

void fill_random(void* buffer, unsigned int length)
{
	int nullfd = open("/dev/urandom", O_RDONLY);
	read(nullfd, (char*)buffer, length);
	close(nullfd);
}

void fill_message_type(void* buffer, MSG_TYPE type)
{
	memcpy(buffer, &type, sizeof(MSG_TYPE));
}

char cs(char* msg, unsigned int len, char result) {
	for(int i = 0; i < len; i++)
	      result += msg[i];
	return result;
}

void phex (const void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}


//inline void xor_big(const char* input1, const char* input2, char* output)
//{

//	static void XOR(block in1[], block in2[], block out[], int sliceLen){
//
//	#ifdef NO_VEC_OPT
//	for (int i=0; i<sliceLen; i++){
//	    out[i] = in1[i] ^ in2[i];
//	 }
//	#else
//
//	    const unsigned int u = 0xFFFFFFFF;
//	    __m256 in1_vec, in2_vec, out_vec;
//	    __m256i ALL_ONES_256 = _mm256_set_epi32(u,u,u,u,u,u,u,u);
//	    __m256i *output_as_int_vec;
//
//	    float *fin1 = (float *)in1;
//	    float *fin2 = (float *)in2;
//	    float *fout = (float *)out;
//
//
//	    for (int i=0; i<sliceLen; i+=8){
//			in1_vec = _mm256_load_ps(fin1 + i);
//				in2_vec = _mm256_load_ps(fin2 + i);
//				out_vec = _mm256_xor_ps(in1_vec, in2_vec);
//				output_as_int_vec = (__m256i* ) &out_vec;
//			_mm256_maskstore_epi32((int *)fout,ALL_ONES_256,(*output_as_int_vec));
//
//	    }
//
//	#endif
//	}
//}
