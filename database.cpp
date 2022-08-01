//
// Created by desktop on 4/23/2022.
//
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

using namespace std;

/** Php like function to split string into array based on character inside string */
vector<string> split(string s, char c){
    int n = s.length();
    int split_count = 0;
    vector<string> output(count(s.begin(),s.end(),c)+1);
    for (int i = 0; i < n; ++i) {
        if(s[i] == c){
            split_count++;
            continue;
        }
        else{
            output[split_count] += s[i];
        }
    }
    return output;
}


map<string,string> load_database(const string& filename){
    string line;
    ifstream my_file(filename);
    map<string,string> out;
    if (my_file.is_open()){
        while (getline(my_file,line)){
            vector<string> user = split(line,':');
            out.insert({user[0],user[1].substr(0,user[1].length()-1)});
        }
        my_file.close();
    }
    else cerr << "Unable to open file!" << endl;
    return out;
}

vector<string> get_users(const map<string,string>& x){
    vector<string> out;
    for (auto & i : x) {
        out.push_back(i.first);
    }
    return out;
}
vector<string> get_passwords(const map<string,string>& x){
    vector<string> out;
    for (auto & i : x) {
        out.push_back(i.second);
    }
    return out;
}

bool x_exist(vector<string> xs,string x){
    return (find(xs.begin(),xs.end(),x) != xs.end());
}
bool password_match(map<string,string> x,string user, string password){
    return (x[user] == password || strcmp(x[user].c_str(),password.c_str())==0);
}
bool password_needed(map<string,string> x,string user){
    return (x[user] != "");
}

bool dirExists(const char *path){
    struct stat info;

    if(stat( path, &info ) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}