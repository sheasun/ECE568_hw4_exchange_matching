#include "mysocket.h"
#include "parser.h"
#include "query_funcs.h"

#include <algorithm>
#include <string>
#include <cmath>

class Server{
public:
    const char * port;
    char client_hostname[256];
    int socket_fd;
public:
    Server():port("12345"){}
    ~Server(){}
    
    void run();
    void connect_db();
    void startServer();

   //void handleRequest(int client_connection_fd, Database db);

    static void * handleRequest(void * msgs);
    static void * doQueue(void * info);
    static void * checkQueue(void * info);

    
};
string firstHandleRequest(int client_connection_fd);
void* secondHandleRequest(int client_connection_fd,string xml);


void send_response(int client_connection_fd, string response);

bool isDouble(const string s);
bool isInteger(const string s);

void printRes(vector<Mytype>&res);

//reference: https://blog.csdn.net/puppylpg/article/details/51260100
template<class out_type,class in_value>
out_type convert(const in_value& t)
{
    stringstream stream;
    out_type result;        //result of convert

    stream << t;            //convey t to stream
    stream >> result;       //write to result
    return result;
}
