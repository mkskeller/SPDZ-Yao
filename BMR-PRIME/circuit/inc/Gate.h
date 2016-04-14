
#ifndef __GATE_H__
#define __GATE_H__

#include <string>
#include <iostream>
#include <boost/thread/mutex.hpp>
#include "common.h"
#include <stdio.h>

#define NO_LAYER (-1)

typedef struct Gate {
	wire_id_t _left;
	wire_id_t _right;
	wire_id_t _out;
	uint8_t _func[4];
	void* _data;

	int _layer;

	inline void init(wire_id_t left, wire_id_t right, wire_id_t out,std::string func) {
		_left = left;
		_right = right;
		_out = out;
		_data = NULL;
		_func[0] = (func[0]=='0')?0:1;
		_func[1] = (func[1]=='0')?0:1;
		_func[2] = (func[2]=='0')?0:1;
		_func[3] = (func[3]=='0')?0:1;
		_layer = NO_LAYER;
	}
	inline uint8_t func(uint8_t left, uint8_t right) {
		return _func[2*left+right];
	}

	void print(int id) {
		printf ("gate %d: l:%d, r:%d, o:%d, func:%d%d%d%d\n", id, _left, _right, _out, _func[0], _func[1], _func[2], _func[3] );
	}

} Gate;

#endif
