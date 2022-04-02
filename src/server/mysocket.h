#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

struct server_t{
    int server_fd;
    int port;
}typedef server_t;


int client_connect_to_server(const char* server_hostname, const char * server_port);

int setup_server(const char*server_port);

int server_accpet(int socket_fd,char* hostname);

server_t setup_server_assign_port();


void *get_in_addr(struct sockaddr *sa);

int accept_connect(int socket_fd);