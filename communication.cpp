//
// Created by desktop on 4/22/2022.
//
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <sys/socket.h>

using namespace std;

// create TCP socket file descriptor
int create_socket(){
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

bool send_message(int socket, string message, int max_bytes){
    message.resize(max_bytes);

    int bytes_sent = send(socket, &message[0], max_bytes,0);
    if (bytes_sent<0) {
        return false;
    }
    else{
        return true;
    }
}

string receive_message(int socket, int bytes){
    string output;
    output.resize(bytes);

    int bytes_received = read(socket,&output[0], bytes);
    if (bytes_received<0){
        return "";
    }
    else{
        string x = output.c_str();
        return x;
    }
}

bool send_file(int socket, char mode, string filename){
    FILE* f = fopen(filename.c_str(),"r");
    char* buffer;
    int el;
    size_t b = 1;
    switch (mode) {
        case 'A':
            el=7;
            break;
        case 'B':
        case 'C':
            el=8;
            break;
    }
    do{
        b = fread(buffer,512,el,f);
        send(socket,buffer,b,0);
    }while(b>0);

}
void receive_file(int socket, char mode, string target){
    FILE* f = fopen(target.c_str(),"w");
    size_t recv_bytes = 1;
    char *buffer;
    int el;
    switch (mode) {
        case 'A':
            el=7;
            break;
        case 'B':
        case 'C':
            el=8;
            break;
    }
    do {
        recv_bytes = read(socket,buffer,el*512);
        fwrite(buffer,el,512,f);
    } while (recv_bytes>0);
}