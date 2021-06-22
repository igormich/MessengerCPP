#ifndef CHAR_SERVER_H
#define CHAR_SERVER_H

#include "TcpServer.h"
#include <iostream>
#include<sstream>
#include <vector>
#include <algorithm>
#include <stdio.h> 
#include<conio.h>
#include <string.h> 
#include <cerrno>

using namespace std;
struct ChatServer {
private:
	vector<std::pair<string, TcpServer::Client*>> clients;
	vector<TcpServer::Client*> newBees;
	void checkNewBees();
	void checkClients();
public:
	void sendToAll(string s, TcpServer::Client* sender);
	void start(int port);
};
#endif //CHAR_SERVER_H
