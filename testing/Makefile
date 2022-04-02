CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -lpthread

all: client


client:client.cpp tinyxml2/tinyxml2.h tinyxml2/tinyxml2.cpp
	$(CC) $(CFLAGS) -o client client.cpp mysocket.cpp tinyxml2/tinyxml2.cpp $(EXTRAFLAGS)
clean:
	rm -f *~ *.o test

clobber:
	rm -f *~ *.o
