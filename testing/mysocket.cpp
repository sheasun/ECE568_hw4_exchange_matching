#include"mysocket.h"

int client_connect_to_server(const char* server_hostname, const char * server_port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(server_hostname, server_port, &host_info, &host_info_list);
    if (status != 0) {
        //cerr << "Error: cannot get address info for host" << endl;
        //cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
        fprintf(stderr,"Error: cannot get address info for host\n");
        fprintf(stderr,"\n");
        return -1;
    } //if


    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        //cerr << "Error: cannot create socket" << endl;
        //cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
        fprintf(stderr,"Error: cannot create socket\n");
        fprintf(stderr,"\n");
        return -1;
    } //if

    //cout << "Connecting to " << server_hostname << " on port " << server_port << "..." << endl;
    //printf("Connecting to %s on port %s ...\n",server_hostname,server_port);

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        //cerr << "Error: cannot connect to socket" << endl;
        //cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
        fprintf(stderr,"Error: cannot connect to socket\n");
        fprintf(stderr,"\n");
        return -1;
    } //if

    freeaddrinfo(host_info_list);
    return socket_fd;
}





int setup_server(const char*server_port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, server_port, &host_info, &host_info_list);
    if (status != 0) {
        //cerr << "Error: cannot get address info for host" << endl;
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot get address info for host\n");
        fprintf(stderr,"\n");
        return -1;
    } //if

    /*
    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        //cerr << "Error: cannot create socket" << endl;
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot create socket\n");
        fprintf(stderr,"\n");
        return -1;
    } //if
    */

    /* avoid repetitive IP address */
    struct addrinfo *p;
    int yes = 1;
    for(p = host_info_list; p!=NULL; p=p->ai_next){
        socket_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
        if(socket_fd<0){
            continue;
        }
        status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(status<0){
            fprintf(stderr,"Error: setsockopt fail\n");
            return -1;
        }
        status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status<0) {
            close(socket_fd);
            continue;
        } //if
        break;
    }

    if (p == NULL) {
        fprintf(stderr,"Error: fail to bind\n");
        return -1;
    }
    /* avoid over */

    status = listen(socket_fd, 100);
    if (status == -1) {
        //cerr << "Error: cannot listen on socket" << endl; 
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot listen on socket\n");
        fprintf(stderr,"\n");
        return -1;
    } //if

    //cout << "Waiting for connection on port " << port << endl;
    //printf("Waiting for connection on port %s\n",server_port);


    freeaddrinfo(host_info_list);
    return socket_fd;
}


void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int server_accpet(int socket_fd,char* hostname){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    char remoteIP[INET6_ADDRSTRLEN];

    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        //cerr << "Error: cannot accept connection on socket" << endl;
        fprintf(stderr,"Error: cannot accept connection on socket\n");
        return -1;
    } //if

    strcpy(hostname,inet_ntop(socket_addr.ss_family,get_in_addr((struct sockaddr*)&socket_addr),remoteIP,INET6_ADDRSTRLEN));
    //strcpy(hostnames[i],inet_ntop(socket_addr.ss_family,get_in_addr((struct sockaddr*)&socket_addr),remoteIP,INET6_ADDRSTRLEN));

    return client_connection_fd;
}

int accept_connect(int socket_fd) {
    int client_connection_fd;
    struct sockaddr_storage player_socket_addr;
    socklen_t player_socket_addr_len = sizeof(player_socket_addr);
    // accept players' connection
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&player_socket_addr, &player_socket_addr_len);
    if (client_connection_fd == -1) {
        fprintf(stderr, "Error: cannot accept connection on socket\n");
        exit(EXIT_FAILURE);
    }
    return client_connection_fd;
}


server_t setup_server_assign_port(){
    server_t ret;
    ret.port = -1;
    ret.server_fd = -1;


    int socket_fd;
    struct sockaddr_in addr;
    socklen_t socklen;

    socket_fd = socket(AF_INET,SOCK_STREAM,0);

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;

    socklen = sizeof(addr);
    if (bind(socket_fd, (struct sockaddr *)&addr, socklen)==-1) {
        fprintf(stderr,"Error: bind error in assign port\n");
        ret.server_fd = -1;
        return ret;
        //return -1;
    }
    
    int status = getsockname(socket_fd, (struct sockaddr *)&addr, &socklen);
    if(status == -1){
        fprintf(stderr,"Error: getsockname fail\n");
        ret.server_fd = -1;
        return ret;
        //return -1;
    }
    //printf("successfully assign port = %u\n", ntohs(addr.sin_port));
    //return ntohs(addr.sin_port);
    

    status = listen(socket_fd, 100);
    if (status == -1) {
        //cerr << "Error: cannot listen on socket" << endl; 
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot listen on socket\n");
        fprintf(stderr,"\n");
        ret.server_fd = -1;
        return ret;
        //return -1;
    } //if

    //cout << "Waiting for connection on port " << port << endl;
    //printf("Waiting for connection on port %s\n",server_port);
    //printf("Waiting for connection on port %d\n",server_port);


    //return socket_fd;
    ret.server_fd = socket_fd;
    ret.port = ntohs(addr.sin_port);
    return ret;
}

/*
int what_is_port(int socket_fd){
    struct sockaddr_in addr;
    socklen_t socklen = sizeof(addr);

    int status = getsockname(socket_fd, (struct sockaddr *)&addr, &socklen);
    if(status == -1){
        fprintf(stderr,"Error: getsockname fail\n");
        return -1;
    }
    printf("successfully assign port = %u\n", ntohs(addr.sin_port));
    return ntohs(addr.sin_port);
}
*/