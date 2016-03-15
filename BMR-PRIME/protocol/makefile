



CPPFLAGS=-g -O3 -msse -msse2 -maes -msse4.1 -mavx2 -c -w --std=c++11 \
		 -D__PRIME_FIELD__ \
		 -I/usr/include \
		 -Iinc \
		 -I../network/inc \
		 -I../circuit_bmr/inc \
		 $(LOCAL_CPPFLAGS)

LDFLAGS=-g -L/usr/lib/x86_64-linux-gnu/ \
	-lboost_system -lpthread  -lboost_thread $(LOCAL_LDFLAGS)

OBJECTS=../circuit_bmr/build/BooleanCircuit.o \
		../circuit_bmr/build/Key.o \
		../network/build/Client.o \
		../network/build/Server.o \
		../network/build/Node.o \
		build/proto_utils.o \
		build/prf.o \
		build/aes.o


test:bin/test
all:bin/tparty bin/party

bin/test:$(OBJECTS) build/test.o
	g++ -Wall $^ -o bin/test $(LDFLAGS)
	
build/test.o:src/test.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

bin/tparty:$(OBJECTS) build/TrustedParty.o
	g++ -Wall $^ -o bin/tparty $(LDFLAGS)

bin/party:$(OBJECTS) build/Party.o
	g++ -Wall $^ -o bin/party $(LDFLAGS)
	
../circuit_bmr/build/BooleanCircuit.o:../circuit_bmr/src/BooleanCircuit.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

../circuit_bmr/build/Key.o:../circuit_bmr/src/Key.cpp
	g++ $(CPPFLAGS) $^ -o $@
	

../network/build/Server.o:../network/src/Server.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
../network/build/Client.o:../network/src/Client.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
../network/build/Node.o:../network/src/Node.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
		 
../network/build/utils.o:../network/src/utils.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	


build/proto_utils.o:src/proto_utils.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

build/TrustedParty.o:src/TrustedParty.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
build/Party.o:src/Party.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

build/aes.o:src/aes.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

build/prf.o:src/prf.cpp
	$(CC) $(CPPFLAGS) $^ -o $@

clean:
	rm build/*
	rm ../circuit_bmr/build/*
	rm ../network/build/*
	rm bin/tparty
	rm bin/party