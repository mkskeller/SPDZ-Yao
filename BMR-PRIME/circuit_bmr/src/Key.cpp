/*
 * Key.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: marcel
 */


#include <string.h>
#include "../../circuit_bmr/inc/Key.h"

#ifndef __PRIME_FIELD__
ostream& operator<<(ostream& o, const Key& key)
{
	return o << key.r;
}

ostream& operator<<(ostream& o, const __m128i& x) {
	o.fill('0');
	o << hex;
	for (int i = 0; i < 2; i++)
	{
		o.width(16);
		o << ((int64_t*)&x)[1-i];
	}
	o << dec;
	return o;
}

Key& Key::operator=(const Key& other) {
	r= other.r;
//	memcpy(&r, &other.r, sizeof(r));
	return *this;
}

bool Key::operator==(const Key& other) {
	__m128i neq = _mm_xor_si128(r, other.r);
	return _mm_test_all_zeros(neq,neq);
}

Key& Key::operator-=(const Key& other) {
	r ^= other.r;
	return *this;
}

Key& Key::operator+=(const Key& other) {
	r ^= other.r;
	return *this;
}

#else //__PRIME_FIELD__ is defined

ostream& operator<<(ostream& o, const Key& key)
{
	return o << key.r;
}

ostream& operator<<(ostream& o, const __uint128_t& x) {
	o.fill('0');
	o << hex;

	for (int i = 0; i < 2; i++)
	{
		o.width(16);
		o << ((int64_t*)&x)[1-i];
	}
	o << dec;
	return o;
}

#endif

