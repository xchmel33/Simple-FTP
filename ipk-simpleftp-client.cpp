//
// Created by Lukas Chmelo on 20.4.2022
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
#include "communication.h"

using namespace std;


int main(int argc, char* argv[]){
    // set default arguments
    string port = "115";
    string ip, working_dir;

    // parse program arguments
    int opt;
    while ((opt = getopt(argc, argv, "h:p:f:")) != -1)
    {
        switch (opt) {
            case 'h':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'f':
                working_dir = optarg;
                break;
            case ':':
                printf("option %c needs a value!\n", opt);
                break;
            case '?':
                printf("unknown option %c\n", optopt);
                break;
        }
    }
    for (; optind < argc; optind++){
        printf("Unknown argument : %s\n", argv[optind]);
    }

    // check for missing arguments
    if (ip.empty()){
        cerr << "User database not specified!" << endl;
        return -1;
    }
    if (working_dir.empty()){
        cerr << "Working directory not specified!" << endl;
        return -1;
    }

    // convert port 'const char*' to 'unsigned long'
    char* end_ptr;
    uint32_t PORT;
    PORT = strtoul(port.c_str(), &end_ptr, 10);

    // create TCP socket file descriptor
    int sock = create_socket();
    if (sock == -1){
        cerr << "Socket creation failed!";
        return -2;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET,ip.c_str(),&server_address.sin_addr) <= 0){
        cerr << "Invalid address/ Address not supported" << endl;
        return -2;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        cerr << "Connection failed!" << endl;
        return -2;
    }

    string filename;
    int bytes=0;
    while(true){

        //send command to server
        string new_command;
        cout << ">";
        getline(cin,new_command);
        send_message(sock,new_command,new_command.length());
        cout << "Client:'"<< new_command <<"' message sent\n" << endl;

        // download file
        if(bytes!=0 && new_command == "SEND"){
            receive_file(sock,'B',working_dir+filename);
        }

        // receive reply from server
        string buffer = receive_message(sock,64);
        cout << "Server: " << buffer.c_str() <<  endl;

        // prepare download from server
        if (new_command.substr(0,4) == "RERT"))){
            filename = new_command.substr(5,new_command.length());
            bytes = atoi(buffer.c_str());
        }

        // close connection if server closes too
        if (buffer == SERVER_EXIT || buffer[0] == '-'){
            cout << "Client: " << CLIENT_EXIT << endl;
            close(sock);
            break;
        }
    }
    return 0;
}