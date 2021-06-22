// SimpleMessenger.cpp: определяет точку входа для приложения.
//


#include "SimpleMessenger.h"
#include "ChatServer.h"
#include "ChatClient.h"
#include "TcpServer.h"
#include "utils.h" 
#include <iostream>
#include<sstream>
#include <vector>
#include <algorithm>
#include <stdio.h> 
#include<conio.h>
#include <string.h> 
#include <cerrno>

using namespace std;



string name = "";
int main(int argc, char* argv[])
{
#ifdef WIN32
	// Initialize Winsock
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 0;
	}
#endif
	if (argc == 2) {
		int port = atoi(argv[1]);
		ChatServer chat;
		chat.start(port);
	}
	else if (argc == 4) {
		name = argv[3];
		int port = atoi(argv[2]);
		ChatClient chatClient(name);
		auto sendFunction = client(argv[1], port, [&chatClient](string s) {chatClient.recv(s); });
		if (sendFunction == NULL)
			return 0;
		chatClient.setSendFunction(sendFunction);
		chatClient.startRead();
	}
	else {
		cout << "Please run with following arguments:" << endl;
		cout << "For server: messenger PORT" << endl;
		cout << "For client: messenger IP PORT NAME" << endl;
		cout << "Usage example: messenger 196.168.0.21 666 User" << endl;
		return 0;
	}

	return 0;
}
