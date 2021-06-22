#ifndef CHAR_CLIENT_H
#define CHAR_CLIENT_H

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

struct ChatClient {
private:
	string name;
	string currentMessage = "";
	StringFunction send;
public:
	ChatClient(const string name);
	void setSendFunction(StringFunction send);
	void recv(const string text);
	void startRead();
};
#endif // CHAR_CLIENT_H
