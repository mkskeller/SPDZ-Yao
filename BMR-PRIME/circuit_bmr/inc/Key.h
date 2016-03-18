/*
 * Key.h
 *
 *  Created on: Oct 27, 2015
 *      Author: marcel
 */

#ifndef COMMON_INC_KEY_H_
#define COMMON_INC_KEY_H_

#include <iostream>
#include <emmintrin.h>
#include <smmintrin.h>
#include <string.h>

#include "proto_utils.h"

using namespace std;

#ifndef __PRIME_FIELD__

class Key {
public:
	__m128i r;

	Key() : r(_mm_set1_epi64x(0)) {}
	Key(long long a) : r(_mm_set1_epi64x(a)) {}
	Key(const Key& other) {r= other.r;}

	Key& operator=(const Key& other);
	bool operator==(const Key& other);

	Key& operator-=(const Key& other);
	Key& operator+=(const Key& other);
};

ostream& operator<<(ostream& o, const Key& key);
ostream& operator<<(ostream& o, const __m128i& x);


#else //__PRIME_FIELD__ is defined

const __uint128_t MODULUS= 0xffffffffffffffffffffffffffffff61;
#define MODP_STR "ffffffffffffffffffffffffffffff61"

#ifdef __PURE_SHE__
#include "mpir.h"
extern mpz_t key_modulo;
void init_modulos();
void init_temp_mpz_t(mpz_t& temp);
#endif

class Key {
public:
	__uint128_t r;

	Key() {r=0;}
	Key(__uint128_t a) {r=a;}
	Key(const Key& other) {r= other.r;}

	Key& operator=(const Key& other){r= other.r; return *this;}
	bool operator==(const Key& other) {return r == other.r;}
	Key& operator-=(const Key& other) {r-=other.r; r%=MODULUS; return *this;}
	Key& operator+=(const Key& other) {r+=other.r; r%=MODULUS; return *this;}

	Key& operator=(const __uint128_t& other){r= other; return *this;}
	bool operator==(const __uint128_t& other) {return r == other;}
	Key& operator-=(const __uint128_t& other) {r-=r; r%=MODULUS; return *this;}
	Key& operator+=(const __uint128_t& other) {r+=other; r%=MODULUS; return *this;}
#if __PURE_SHE__
	inline __uint128_t sqr(mpz_t& temp) {
		temp->_mp_size = 2;
		*((__int128*)temp->_mp_d) = r;
		mpz_mul(temp, temp, temp);
		mpz_tdiv_r(temp, temp, key_modulo);
		__uint128_t ret = *((__int128*)temp->_mp_d);
		return ret;
	}
	inline Key& sqr_in_place(mpz_t& temp) {
		temp->_mp_size = 2;
		*((__int128*)temp->_mp_d) = r;
		mpz_mul(temp, temp, temp);
		mpz_tdiv_r(temp, temp, key_modulo);
		r = *((__int128*)temp->_mp_d);
		return *this;
	}
#else
	inline Key& sqr() {r=r*r; r%=MODULUS; return *this;}
#endif
	inline void adjust() {r=r%MODULUS;}
};

ostream& operator<<(ostream& o, const Key& key);
ostream& operator<<(ostream& o, const __uint128_t& x);


#endif

#endif /* COMMON_INC_KEY_H_ */
