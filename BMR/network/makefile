



CPPFLAGS=-g -O3 -c -w --std=c++11 \
		 -I/usr/include \
		 -Iinc \
		 $(LOCAL_CPPFLAGS)

LDFLAGS=-g -L/usr/lib/x86_64-linux-gnu/ \
	-lboost_system -lpthread  -lboost_thread $(LOCAL_LDFLAGS)

OBJECTS=build/test.o \
		build/utils.o \
		build/Server.o \
		build/Client.o \
		build/Node.o
	
	
all:bin/net

bin/net:$(OBJECTS)
	g++ -Wall $^ -o bin/net $(LDFLAGS)
	
	
build/Server.o:src/Server.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
build/Client.o:src/Client.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
build/Node.o:src/Node.cpp
	$(CC) $(CPPFLAGS) $^ -o $@	 

build/test.o:test/test.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
build/utils.o:src/utils.cpp
	$(CC) $(CPPFLAGS) $^ -o $@
	
	
clean:
	rm build/* bin/net