CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -lpthread

all: server client

server: main.cpp query_funcs.h query_funcs.cpp parser.h parser.cpp mysocket.h mysocket.cpp server.h server.cpp tinyxml2/tinyxml2.h tinyxml2/tinyxml2.cpp
	$(CC) $(CFLAGS) -o server main.cpp query_funcs.cpp parser.cpp mysocket.cpp server.cpp tinyxml2/tinyxml2.cpp $(EXTRAFLAGS)

client:client.cpp tinyxml2/tinyxml2.h tinyxml2/tinyxml2.cpp
	$(CC) $(CFLAGS) -o client client.cpp mysocket.cpp tinyxml2/tinyxml2.cpp $(EXTRAFLAGS)
clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o
