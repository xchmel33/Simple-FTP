#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <sys/socket.h>
#include "communication.h"
#include "database.h"

using namespace std;


int main(int argc,char* argv[]) {

    // set default arguments
    string interface = "0.0.0.0";
    string port = "115";
    string database, working_dir = "";

    // parse program arguments
    int opt;
    while ((opt = getopt(argc, argv, "i:p:u:f:")) != -1)
    {
        switch (opt) {
            case 'i':
                interface = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'u':
                database = optarg;
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
    if (database == ""){
        cerr << "User database not specified!" << endl;
        return -1;
    }
    if (working_dir == ""){
        cerr << "Working directory not specified!" << endl;
        return -1;
    }

    // convert port 'const char*' to 'unsigned long'
    char* end_ptr;
    uint32_t PORT;
    PORT = strtoul(port.c_str(), &end_ptr, 10);

    // set variables for communication
    int new_sock;
    int o = 1;

    // create TCP socket file descriptor
    int sock = create_socket();
    if (sock == -1){
        cerr << "Socket creation failed!";
        return -2;
    }

    // set socket options
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o)) < 0){
        cerr << "Reuse address option failed" << endl;
        return -2;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &o, sizeof(o)) < 0){
        cerr << "Reuse port option failed" << endl;
        return -2;
    }

    // set address struct interface and port
    struct sockaddr_in address;
    int address_len = sizeof(address);
    address.sin_family = AF_INET;
    inet_aton(interface.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);


    // bind socket port
    if (bind(sock, (struct sockaddr*)&address, address_len) < 0){
        cerr << "Bind failed!" << endl;
        return -2;
    }

    // listen
    if (listen(sock, 3) < 0){
        cerr << "Listen failed!" << endl;
        return -2;
    }
    cout << "Server: Listening on " << interface << ":" << port << endl;

    // accept client commands
    new_sock = accept(sock, (struct sockaddr*)&address, (socklen_t*)&address_len);
    if (new_sock < 0){
        cerr << "Accept failed!" << endl;
        return -2;
    }

    string user, password;
    char type = 'A';
    bool logged_in = false, accept_pass = false;
    map<string, string> db = load_database(database);
    vector<string> users = get_users(db);
//    for (auto & i : keys){
//        cout << "User: " << i << " Password: " << users[i] << endl;
//    }
    while(true){

        // wait for command
        string buffer;
        buffer = receive_message(new_sock,32);
        string command = buffer.substr(0,4), args = buffer.substr(5,buffer.lenght()), reply;

        // process command
        if (command=="DONE"){
            send_message(new_sock,SERVER_EXIT, 32);
            cout << "Server: " << SERVER_EXIT << endl;
            close(new_sock);
            close(sock);
            break;
        }
        else if(command=="USER"){
            if (buffer.length() <= 4){
                reply = "-Invalid user-id, try again";
            } else {
                user = args;
                if (x_exist(users,user)){
                    reply.append("+");
                    reply.append(user);
                    reply.append(" ok, send password");
                    accept_pass = true;
                } else{
                    reply = "-Invalid user-id, try again";
                }
            }
        }
        else if(command == "ACCT"){
            if (buffer.length() <= 4){
                reply = "-Invalid account, try again";
            } else {
                user = args;
                if (x_exist(users,user)){
                    if (password_needed(db,user)){
                        reply ="+Account valid, send password";
                        accept_pass = true;
                    }
                    else{
                        reply = "! Account valid, logged-in";
                        logged_in = true;
                    }
                } else{
                    reply = "-Invalid account, try again";
                }
            }
        }
        else if(command=="PASS") {
            if (accept_pass) {
                if (buffer.length() <= 4) {
                    reply = "-Wrong password, try again";
                } else{
                    password = args;
                    cout << "Client password: " << password << endl;
                    if (password_match(db, user, password)) {
                        reply = "! Logged in";
                        logged_in = true;
                    }
                    else{
                        reply = "-Wrong password, try again";
                    }
                }
            }
            else{
                reply = "+Send account";
            }
        }
        else if(logged_in){

            if(command=="TYPE") {
                type=(char)args.c_str()[0];
                reply.append("+Using ");
                switch (type) {
                    case 'A':
                        reply.append("Ascii ");
                        break;
                    case 'B':
                        reply.append("Binary ");
                        break;
                    case 'C':
                        reply.append("Continuous ");
                        break;
                    default:
                        type = 'B';
                        reply.append("Binary ");
                        break;
                }
                reply.append("mode");
            }
            else if(command=="LIST") {
                string path = args.substr(2,args.length());
                vector<string> files;
                for (const auto & entry : filesystem::directory_iterator(working_dir+path)){
                    reply.append(entry.path());
                    reply.append("\n");
                }

//                // verbose directory listing
//                if (args[0] == 'V'){
//                }
//                // standard formatted directory listing -> default
//                else{
//
//                }


            }
            else if(command=="CDIR") {
                string xx = args.substr(0,args.length()-1);
                DIR* dir = opendir(xx);
                if (dir){
                    // user has access
                    if (xx == user || xx == password){
                        reply = "!Changed working directory to ";
                        reply.append(xx);
                        working_dir = xx;
                    }

                    // user needs access
                    else{
                        string msg = "+directory ok, send account/password", usr, pwd;
                        send_message(new_sock,msg,msg.size());
                        while(true){
                            string res = receive_message(new_sock,64);
                            if (res.substr(0,4) == "PASS"){
                                string px = res.substr(5,res.length());
                                if (usr.empty()){
                                    vector<string> passwords = get_passwords(db);
                                    if (x_exist(passwords,px)){
                                        msg = "+password ok, send account";
                                        pwd = password;
                                    }
                                    else{
                                        reply = "-invalid password";
                                        break;
                                    }
                                }
                                else if (password_match(db,usr,px)){
                                    reply = "!Changed working directory to ";
                                    reply.append(xx);
                                    working_dir = xx;
                                    break;
                                }
                            }
                            else if(res.substr(0,4) == "ACCT"){
                                string ux = res.substr(5,res.length());
                                if (pwd.empty()){
                                    if (x_exist(users,ux)){
                                        if (password_needed(db,ux)){
                                            msg = "+account ok, send password";
                                            usr = ux;
                                        }
                                        else{
                                            reply = "!Changed working directory to ";
                                            reply.append(xx);
                                            working_dir = xx;
                                            break;
                                        }
                                    }
                                    else{
                                        reply = "-invalid password";
                                        break;
                                    }
                                }
                                else if (password_match(db,res.substr(0,5), pwd)){
                                    reply = "!Changed working directory to ";
                                    reply.append(xx);
                                    working_dir = xx;
                                    break;
                                }
                                else{
                                    reply = "-invalid password";
                                    break;
                                }
                            }
                            send_message(new_sock,msg,msg.size());
                        }

                    }
                }
                else{
                    reply = "-Can't connect to directory because: directory does not exist";
                }

            }
            else if(command=="KILL") {
                if (remove(args) != 0){
                    reply = "-Not deleted because file does not exist"
                }
                else{
                    reply = "+";
                    reply.append(args);
                    reply.append(" deleted");
                }
            }
            else if(command=="NAME"){
                ifstream file(args);
                if (file.good()){
                    send_message(new_sock,"+File exists",12);
                    string res = receive_message(new_sock,64), new_name = res.substr(5,res.length());
                    if (res.substr(0,4) == "TOBE" && rename(args,new_name)==0){
                        reply.append(args);
                        reply.append(" renamed to ");
                        reply.append(new_name);
                    }
                    else{
                        reply = "-File wasn't renamed because invalid command, use TOBE <new_file_spec>";
                    }
                }
                else{
                    reply = "-Can't find ";
                    reply.append(args);
                }

            }
            else if(command=="RETR"){
                ifstream file(args, ios::binary | ios::ate);
                if (file.good()){
                    string msg = file.tellq();
                    send_message(new_sock,msg,msg.size());
                    string res = receive_message(new_sock,4);
                    if (res == "SEND" && send_file(new_sock,type,args)){
                        reply = "File transfer completed";
                    }
                    else if (res == "STOP"){
                        reply = "+ok, RETR aborted";
                    }
                }

                }

            else{
                cout << "-Unknown command " << command << endl;
            }
        }
        else{
            if(!logged_in){
                reply = "+send account/password";
            }
            else{
                cout << "-Unknown command " << command << endl;
            }
        }


        cout << "Client: " << buffer << endl;
        if (logged_in) cout << "Logged in! " << endl;

        // send reply to client
        send_message(new_sock,reply,reply.size());

        // [RFC 913] If the server send back a '-' response it will also close the connection, exceptions: STOR, RETR
        if (reply[0] == '-' && command != "STOR" && command != "RETR"){
            close(new_scok);
            close(sock);
            cout << "Server: Closing connection!" << endl;
            break;
        }
        else{
            cout << "Server: Reply sent!" << endl;
        }
        buffer = "";
    }
    return 0;
}