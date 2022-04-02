#include "server.h"
#include "pthread.h"

#include <thread>
#include <atomic>
#include "concurrentqueue/concurrentqueue.h"

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

struct client_t {
  int client_fd;
  Database db;
  int socket_fd;
  char client_hostname[256];
};
typedef struct client_t client;

atomic_int thread_num = {0};
moodycamel::ConcurrentQueue<pair<int,string>>q;
void * Server::doQueue(void * info){
    int*socket_fdd = (int*)info;
    int socket_fd = *socket_fdd;
    while(1){
        //new connection
        //if new request ,q.enqueue{xml,fd}
        
        cout<<"socket_fd = "<<socket_fd<<endl;
        int client_connection_fd = accept_connect(socket_fd);
        cout<<"why not here"<<endl;
        if(client_connection_fd < 0){
            cout<<"wrong client_fd"<<endl;
            continue;
        }
        
        string xml = firstHandleRequest(client_connection_fd);
        if(xml == ""){cout<<"cannot go"<<endl;continue;}
        thread_num.fetch_add(1,memory_order_relaxed);
        q.enqueue({client_connection_fd,xml});
    }
    return NULL;
}
void * Server::checkQueue(void * info){
    while(1){
        if(thread_num < 30){
            //cout<<thread_num<<endl;
            //q.try_dequeue
            //handle request
            pair<int,string>ret;
            bool isdequeue = q.try_dequeue(ret);
            if(isdequeue){secondHandleRequest(ret.first,ret.second);}
        }
    }
    return NULL;
}

void Server::run(){
    startServer();
    //pthread_create : atomic queue
    //pthread_create : check the currentThread is full or not
    // if not full, handle else ??
    
    /*
    int client_connection_fd = accept_connect(socket_fd);
    if(client_connection_fd < 0){
        cout<<"wrong client_fd"<<endl;
    }else{
        string xml = firstHandleRequest(client_connection_fd);
        if(xml == ""){cout<<"cannot go"<<endl;}else{
            thread_num.fetch_add(1,memory_order_relaxed);
            q.enqueue({client_connection_fd,xml});
        }
    }*/
    pthread_t thread1;
    pthread_t thread2;


    pthread_create(&thread1,NULL,doQueue,&socket_fd);
    pthread_create(&thread2,NULL,checkQueue,NULL);

    while(1){}
    /*
    while(1){
        //
        int client_connection_fd = server_accpet(socket_fd,client_hostname);

        if(client_connection_fd < 0){cout<<"wrong client_fd"<<endl;continue;}
        client * client_msg = new client();
        client_msg->client_fd = client_connection_fd;
        
        pthread_t thread;
        
        if(thread_num > 20)while(1){
            if(thread_num < 20)break;
        }
        thread_num.fetch_add(1,memory_order_relaxed);
        pthread_create(&thread,NULL,handleRequest, client_msg);
        
        //
        
    }*/

}
/*
void Server::connect_db(){
    try{
        Database db;
        db.init_db();
        db.disconnect();
    } catch (const std::exception &e) {
        return;
    }
    return;
}
*/

void Server::startServer(){
    int status;
    //int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        //cerr << "Error: cannot get address info for host" << endl;
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot get address info for host\n");
        fprintf(stderr,"\n");
        exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }
    /* avoid over */

    status = listen(socket_fd, 100);
    if (status == -1) {
        //cerr << "Error: cannot listen on socket" << endl; 
        //cerr << "  (" << hostname << "," << port << ")" << endl;
        fprintf(stderr,"Error: cannot listen on socket\n");
        fprintf(stderr,"\n");
        exit(EXIT_FAILURE);
    } //if

    //cout << "Waiting for connection on port " << port << endl;
    //printf("Waiting for connection on port %s\n",server_port);


    freeaddrinfo(host_info_list);
    //return socket_fd;
}

string firstHandleRequest(int client_connection_fd){
    /* receive */
    char buffer[65535] = {0};
    /*
    pthread_mutex_lock(&mymutex);
    cout<<"client_connection_fd = "<< client_connection_fd <<endl;
    pthread_mutex_unlock(&mymutex);*/
    int len = recv(client_connection_fd,buffer,65535,0);
    if(len <= 0){        
        fprintf(stderr,"Error: fail to receive\n");
        return "";
    }
     
    int index = 0;
    while(buffer+index<buffer+len){
        if(buffer[index]=='\n'){break;}
        index++;
    }
    
    vector<char>xml_msg(buffer+index,buffer+len);
    string xml(xml_msg.begin(),xml_msg.end());

    //pthread_mutex_lock(&mymutex);
    cout<<"=============xml request=================="<<endl;
    cout<<xml<<endl;
    cout<<"=============xml end=================="<<endl;
    return xml;
}

void *secondHandleRequest(int client_connection_fd,string xml){
    Database db;
    db.connect_db();

    /* parsing */
    vector<Mytype>res;
    int ret = myXmlParser(xml,res);
    string firstresponse;
    if(ret == -1){
        //cout<<"?"<<endl;
        firstresponse = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><error>Wrong XML Format</error>";
        send_response(client_connection_fd,firstresponse);
        return NULL;
    }
    
    pthread_mutex_lock(&mymutex);
    /* database */
    for(int i = 0;i<res.size();++i){
        //cout<<"everything ok?"<<" i = "<<i <<endl;
        if(res[i].type == "account"){
            //cout<<"create account, id = "<<r.account_id << " balance = " <<r.account_balance<<endl;
            if (isInteger(res[i].account_id) && isDouble(res[i].account_balance)) {
                int account_id = convert<int>(res[i].account_id);
                double account_balance = convert<double>(res[i].account_balance);string msg = "";
                db.create_account(account_id, account_balance, msg);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            } else {
                string msg = "an error from r.account_id/r.account_balance type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        } else if(res[i].type == "symbol"){
            string sym = res[i].symbol;
            for(int j = 0; j < res[i].symbol_id.size(); ++j) {
                if (isInteger(res[i].symbol_shares[j]) && isDouble(res[i].symbol_shares[j])) {
                    int account_id = convert<int>(res[i].symbol_id[j]);
                    double amount = convert<double>(res[i].symbol_shares[j]);string msg = "";
                    db.create_position(account_id, sym, amount,msg);
                    if(msg != ""){
                        res[i].error_arr.push_back(true);
                        res[i].error_arr_msg.push_back(msg);
                    }else{
                        res[i].error_arr.push_back(false);
                        res[i].error_arr_msg.push_back(msg);
                    }
                } else {
                    string msg = "an error from symbol: r.symbol_id/r.symbol_shares";
                    res[i].error_arr.push_back(true);
                    res[i].error_arr_msg.push_back(msg);
                }
            }
        } else if (res[i].type == "order"){
            if (isInteger(res[i].account_id) && isDouble(res[i].order_amount) && isDouble(res[i].order_limit)) {
                string sym = res[i].order_symbol;
                double amount = convert<double>(res[i].order_amount);
                int account_id = convert<int>(res[i].account_id);
                double price = convert<double>(res[i].order_limit);
                string msg = "";
                int transaction_id = db.create_order(account_id, sym, amount, price, msg);
                res[i].transaction_id = convert<string>(transaction_id);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            } else {
                string msg = "an error from \"order\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }else if(res[i].type == "query"){
            if(isInteger(res[i].account_id)&& isInteger(res[i].transaction_id)){
                int account_id = convert<int>(res[i].account_id);
                int transaction_id = convert<int>(res[i].transaction_id);
                string msg ="";
                db.query(account_id,transaction_id,res[i].open_shares,res[i].cancel_shares,res[i].cancel_time,res[i].execute_shares,res[i].execute_price,res[i].execute_time,msg);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            }else{
                string msg = "an error from \"query\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }else if(res[i].type == "cancel"){
            if(isInteger(res[i].account_id)&& isInteger(res[i].transaction_id)){
                int account_id = convert<int>(res[i].account_id);
                int transaction_id = convert<int>(res[i].transaction_id);
                string msg = "";
                db.cancel_order(account_id, transaction_id, res[i].cancel_shares,res[i].cancel_time,res[i].execute_shares,res[i].execute_price,res[i].execute_time,msg);
                if(msg != ""){
                    //cout<<"cancel:fail"<<endl;
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            }else{
                string msg = "an error from \"cancel\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }
    }
    pthread_mutex_unlock(&mymutex);

    printRes(res);//testing
    string response = "";
    ResToXml(res,response);
    send_response(client_connection_fd,response);
    
    pthread_mutex_lock(&mymutex);
    cout<<"------"<<endl;
    cout<<response<<endl;
    cout<<"------"<<endl;
    pthread_mutex_unlock(&mymutex);
    close(client_connection_fd);

    thread_num.fetch_sub(1,memory_order_relaxed);
    //delete client_msg;
    return NULL;
}
void * Server::handleRequest(void * msgs){
    pthread_mutex_lock(&mymutex);
    client* client_msg = (client * )msgs;
    int client_connection_fd = client_msg->client_fd;
    //cout<<"inner client_connetion_fd = "<<client_connection_fd<<endl;
    pthread_mutex_unlock(&mymutex);
    
    Database db;
    db.connect_db();

    /* receive */
    char buffer[65535] = {0};
    /*
    pthread_mutex_lock(&mymutex);
    cout<<"client_connection_fd = "<< client_connection_fd <<endl;
    pthread_mutex_unlock(&mymutex);*/
    int len = recv(client_connection_fd,buffer,65535,0);
    if(len <= 0){        
        fprintf(stderr,"Error: fail to receive\n");
        return NULL;
    }
     
    int index = 0;
    while(buffer+index<buffer+len){
        if(buffer[index]=='\n'){break;}
        index++;
    }
    
    vector<char>xml_msg(buffer+index,buffer+len);
    string xml(xml_msg.begin(),xml_msg.end());

    //pthread_mutex_lock(&mymutex);
    
    cout<<"=============xml request=================="<<endl;
    cout<<xml<<endl;
    cout<<"=============xml end=================="<<endl;
    
 
    /* parsing */
    vector<Mytype>res;
    int ret = myXmlParser(xml,res);
    string firstresponse;
    if(ret == -1){
        cout<<"?"<<endl;
        firstresponse = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><error>Wrong XML Format</error>";
        send_response(client_connection_fd,firstresponse);
        return NULL;
    }
    
    pthread_mutex_lock(&mymutex);
    /* database */
    for(int i = 0;i<res.size();++i){
        //cout<<"everything ok?"<<" i = "<<i <<endl;
        if(res[i].type == "account"){
            //cout<<"create account, id = "<<r.account_id << " balance = " <<r.account_balance<<endl;
            if (isInteger(res[i].account_id) && isDouble(res[i].account_balance)) {
                int account_id = convert<int>(res[i].account_id);
                double account_balance = convert<double>(res[i].account_balance);string msg = "";
                db.create_account(account_id, account_balance, msg);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            } else {
                string msg = "an error from r.account_id/r.account_balance type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        } else if(res[i].type == "symbol"){
            string sym = res[i].symbol;
            for(int j = 0; j < res[i].symbol_id.size(); ++j) {
                if (isInteger(res[i].symbol_shares[j]) && isDouble(res[i].symbol_shares[j])) {
                    int account_id = convert<int>(res[i].symbol_id[j]);
                    double amount = convert<double>(res[i].symbol_shares[j]);string msg = "";
                    db.create_position(account_id, sym, amount,msg);
                    if(msg != ""){
                        res[i].error_arr.push_back(true);
                        res[i].error_arr_msg.push_back(msg);
                    }else{
                        res[i].error_arr.push_back(false);
                        res[i].error_arr_msg.push_back(msg);
                    }
                } else {
                    string msg = "an error from symbol: r.symbol_id/r.symbol_shares";
                    res[i].error_arr.push_back(true);
                    res[i].error_arr_msg.push_back(msg);
                }
            }
        } else if (res[i].type == "order"){
            if (isInteger(res[i].account_id) && isDouble(res[i].order_amount) && isDouble(res[i].order_limit)) {
                string sym = res[i].order_symbol;
                double amount = convert<double>(res[i].order_amount);
                int account_id = convert<int>(res[i].account_id);
                double price = convert<double>(res[i].order_limit);
                string msg = "";
                int transaction_id = db.create_order(account_id, sym, amount, price, msg);
                res[i].transaction_id = convert<string>(transaction_id);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            } else {
                string msg = "an error from \"order\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }else if(res[i].type == "query"){
            if(isInteger(res[i].account_id)&& isInteger(res[i].transaction_id)){
                int account_id = convert<int>(res[i].account_id);
                int transaction_id = convert<int>(res[i].transaction_id);
                string msg ="";
                db.query(account_id,transaction_id,res[i].open_shares,res[i].cancel_shares,res[i].cancel_time,res[i].execute_shares,res[i].execute_price,res[i].execute_time,msg);
                if(msg != ""){
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            }else{
                string msg = "an error from \"query\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }else if(res[i].type == "cancel"){
            if(isInteger(res[i].account_id)&& isInteger(res[i].transaction_id)){
                int account_id = convert<int>(res[i].account_id);
                int transaction_id = convert<int>(res[i].transaction_id);
                string msg = "";
                db.cancel_order(account_id, transaction_id, res[i].cancel_shares,res[i].cancel_time,res[i].execute_shares,res[i].execute_price,res[i].execute_time,msg);
                if(msg != ""){
                    //cout<<"cancel:fail"<<endl;
                    res[i].is_error = true;
                    res[i].error_msg = msg;
                }
            }else{
                string msg = "an error from \"cancel\": variable type";
                res[i].is_error = true;
                res[i].error_msg = msg;
            }
        }
    }
    pthread_mutex_unlock(&mymutex);

    //printRes(res);//testing
    string response = "";
    ResToXml(res,response);
    send_response(client_connection_fd,response);
    
    pthread_mutex_lock(&mymutex);
    cout<<"------"<<endl;
    cout<<response<<endl;
    cout<<"------"<<endl;
    pthread_mutex_unlock(&mymutex);
    close(client_connection_fd);

    thread_num.fetch_sub(1,memory_order_relaxed);
    db.disconnect();
    delete client_msg;
    return NULL;
    
}




// check if it is double type
bool isDouble(const string s) {
    char * p = NULL;
    double d = strtod(s.c_str(), &p);
    return d != HUGE_VAL && *p == '\0' && p != s.c_str();
}

bool isInteger(const string s) {
    bool check = all_of(begin(s), end(s), ::isdigit);
    return check;
}
//


//print res(for testing)
void printRes(vector<Mytype>&res){
    for(int i = 0;i<res.size();++i){
        cout << res[i].type;
        if(res[i].type=="account"){
            cout<<" id = "<<res[i].account_id;
            cout<<" balance = "<<res[i].account_balance;
            cout<<" is_error = "<<res[i].is_error<<endl;
        }
        else if (res[i].type == "symbol"){
            cout<<" = "<< res[i].symbol<<endl;
            for(int j = 0; j < res[i].symbol_id.size();++j){
                cout<<"id = "<<res[i].symbol_id[j] <<" ";
                cout<<"shares = "<<res[i].symbol_shares[j]<<endl;
                cout<<"is_error = " << res[i].error_arr[j]<<endl;
                cout<<"is_error_msg = " << res[i].error_arr_msg[j]<<endl;
            }
        }else if (res[i].type == "order"){
            cout<< " symbol = "<<res[i].order_symbol;
            cout<< " amount = "<<res[i].order_amount;
            cout<< " limit = "<<res[i].order_limit<<endl;
        }else if (res[i].type == "query"){
            cout<<" transaction id = "<<res[i].transaction_id <<endl;
        }else if (res[i].type == "cancel"){
            cout<<" transaction id = "<<res[i].transaction_id <<endl;
        }else if(res[i].type==" error "){
            cout<<" error "<<endl;
        }
        cout << endl;
    }
}

void send_response(int client_connection_fd, string response){
    int len = send(client_connection_fd,response.data(),response.length(),0);
    if(len < 0){
        fprintf(stderr,"Error: fail to send\n");
    }
}