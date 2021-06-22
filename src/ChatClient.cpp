#include "ChatClient.h"

ChatClient::ChatClient(const string name) : name(name) {}
void ChatClient::setSendFunction(StringFunction send) {
	this->send = send;
	send(name);
}
void ChatClient::recv(const string text) {
	printFromStart(text, currentMessage);
}
void ChatClient::startRead() {
	string InputPrompt = "You:";
	currentMessage = InputPrompt;
	cout << InputPrompt;
	while (true) {
		char c = 0;
		while (c != 13) {
			c = getch();
			if (c == 3)
				exit(0);
			//cout << (int)c << endl;
			if (c == -32) {//skip control keys
				getch();
				continue;
			}
			if (c == 8) {
				if (currentMessage.size() > InputPrompt.size())
					currentMessage = currentMessage.substr(0, currentMessage.size() - 1);
			}
			if (iswgraph(c) || (c == ' ')) {
				currentMessage += c;
			}
			fillLine();
			cout << '\r' << currentMessage;
		}
		if (currentMessage.size() > InputPrompt.size()) {
			cout << endl << InputPrompt;
			auto messageToSend = currentMessage.substr(InputPrompt.length());
			currentMessage = InputPrompt;
			send(messageToSend);
		}
	}
}
