


CPPFLAGS=-g -c -w -O3 --std=c++11 \
							 -I/usr/include \
							 -I/usr/include/boost \
							 -Iinc \
							 $(LOCAL_CPPFLAGS)

LDFLAGS=-g -L/usr/lib/x86_64-linux-gnu/ \
	-lboost_system -lpthread  -lboost_thread $(LOCAL_LDFLAGS)

OBJECTS=build/test.o \
		build/BooleanCircuit.o

all:bin/bc

bin/bc:$(OBJECTS)
	g++ -Wall $^ -o bin/bc $(LDFLAGS)

build/test.o:test/test.cpp
	$(CC) $(CPPFLAGS) $^ -o $@ 
build/BooleanCircuit.o:src/BooleanCircuit.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
clean:
	rm build/* bin/bc


