//
// Created by desktop on 4/23/2022.
//

#ifndef IPK_SIMPLEFTP_DATABASE_H
#define IPK_SIMPLEFTP_DATABASE_H
std::vector<std::string> split(std::string s, char c);
std::map<std::string,std::string> load_database(const std::string& filename);
std::vector<std::string> get_users(const std::map<std::string,std::string>& x);
std::vector<std::string> get_passwords(const std::map<std::string,std::string>& x);
bool x_exist(std::vector<std::string> xs,std::string x);
bool password_match(std::map<std::string,std::string> x,std::string user,std::string password);
bool password_needed(std::map<std::string,std::string> x,std::string user);
bool dirExists(const char* path);
#endif //IPK_SIMPLEFTP_DATABASE_H
