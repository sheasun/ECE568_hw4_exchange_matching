#include "mysocket.h"
#include <string>
#include <iostream>
#include <vector>
#include "xmlGenerator.h"

#include "pthread.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
const char * hostname = "0.0.0.0";const char* port = "12345";

void * handleRequest(void * msg){
    
    /*
    int * client_msg = (int *)msg;
    int client_fd = *client_msg;
    */
    
    
    int client_fd = client_connect_to_server(hostname,port);
    if(client_fd < 0){
        fprintf(stderr,"Error: fail to connect to server\n");
        exit(EXIT_FAILURE);
    }
    

    xmlGenerator myXmlGenerator;
    string xml = myXmlGenerator.generateXml();
    //string xml = "0\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<transactions id=\"4\">\n</transactions>\n";
    
    /*string xml = "2800000\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml+="<transactions id=\"4\">\n";
    xml+="<order sym=\"SYM\" amount=\"300\" limit=\"125\"/>\n";
    xml+="<order sym=\"SYM\" amount=\"-100\" limit=\"130\"/>\n";
    xml+="<order sym=\"SYM\" amount=\"200\" limit=\"127\"/>\n";
    xml+="<order sym=\"SYM\" amount=\"-500\" limit=\"128\"/>\n";
    xml+="<order sym=\"SYM\" amount=\"-200\" limit=\"140\"/>\n";
    xml+="<order sym=\"SYM\" amount=\"400\" limit=\"125\"/>\n";
    xml+="</transactions>\n";
    */
   


    

    
    //send xml
    
    int len = send(client_fd,xml.data(),xml.length(),0);
    if(len <= 0){
        fprintf(stderr,"Error: fail to send\n");
        return NULL;
    }
    
    pthread_mutex_lock(&mutex);
    cout<<"send"<<endl;
    cout<<"inner client_fd = "<<client_fd<<endl;
    cout <<"============my request============"<<endl;
    cout << xml << endl;
    cout<<"=============my request end========="<<endl;
    pthread_mutex_unlock(&mutex);
    
    

    //receive xml
    
    char buffer[65535];
    int recv_len = recv(client_fd,buffer,65535,0);
    if(recv_len <= 0){
        fprintf(stderr,"cannot receive");
        return NULL;
    }
    //cout<<"where is the receive"<<endl;
    vector<char>recv_msg(buffer,buffer+recv_len);
    string xml_response(recv_msg.begin(),recv_msg.end());
    
    pthread_mutex_lock(&mutex);
    cout<<"=================xml response=============="<<endl;
    cout<<xml_response<<endl;
    cout<<"=================xml end=============="<<endl;
    cout<<"receive ends"<<endl;
    pthread_mutex_unlock(&mutex);

    close(client_fd);

    return NULL;
}

int main(){
    //const char * hostname = "67.159.89.93";const char* port = "12345";
    int thread_num = 0;
    for(int i = 0; i < 1000; ++i){
        
        //pthread_create
        pthread_t thread;
        int ret = pthread_create(&thread,NULL,handleRequest,NULL);
        if(ret != 0){
            cout<<"pthread create error"<<endl;
        }
        /*
        //no pthread_create : first become a client
        handleRequest(NULL);*/
        
        
    }
    sleep(2);
    return 0;
}
