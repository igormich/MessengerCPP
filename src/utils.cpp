
#include "utils.h"
#include <iostream>
#include<sstream>
#include <vector>
#include <algorithm>
#include <stdio.h> 
#include<conio.h>
#include <string.h> 
#include <cerrno>
#include <WinSock2.h>
#include <cstdint>
#include <functional>
#include <thread>
#include "TcpServer.h"

using namespace std;

void fillLine() {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	short width = csbiInfo.dwSize.X;
	cout << '\r' << string(width - 1, ' ');
}

void printFromStart(string text, string oldText) {
	fillLine();
	cout << '\r' << text << endl << oldText;
}
StringFunction client(char* ip, int port, StringFunction recvCallback) {
	UINT_PTR sock = 0;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return NULL;
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.S_un.S_addr = inet_addr(ip);
	auto code = connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
	if (code < 0)
	{
		cout << ip << " ";
		cout << port << " ";
		cout << WSAGetLastError() << " ";
		cout << "Connection Failed " << code;
		return NULL;
	}
	thread([sock, recvCallback]() {
		while (true) {
			char buffer[buffer_size] = { 0 };
			auto nReadBytes = recv(sock, buffer, buffer_size, 0);
			if (nReadBytes == SOCKET_ERROR) {
				recvCallback(string("Server close connection"));
				exit(0);
			}
			if (nReadBytes > 0) {
				recvCallback(string(buffer));
			}
		}
		}).detach();
		return [sock](string s) {
			send(sock, s.c_str(), s.length() + 1, 0);
		};
}
