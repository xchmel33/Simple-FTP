//
// Created by desktop on 4/22/2022.
//

#ifndef IPK_SIMPLEFTP_COMMUNICATION_H
#define IPK_SIMPLEFTP_COMMUNICATION_H
#define SERVER_EXIT "+MIT-XX closing connection"
#define CLIENT_EXIT "DONE"
bool send_message(int socket, std::string message, int max_bytes);
int create_socket();
std::string receive_message(int socket, int max_bytes);
bool send_file(int socket, char mode, std::string filename);
void receive_file(int socket, char mode, std::string target);
#endif //IPK_SIMPLEFTP_COMMUNICATION_H
