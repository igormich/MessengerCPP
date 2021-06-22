#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include<sstream>
#include <vector>
#include <algorithm>
#include <stdio.h> 
#include<conio.h>
#include <string.h> 
#include <cerrno>
#include <cerrno>
#include <functional>

using namespace std;

typedef function<void(string)> StringFunction;

void fillLine();

void printFromStart(string text, string oldText);

StringFunction client(char* ip, int port, StringFunction recvCallback);

#endif // UTILS_H