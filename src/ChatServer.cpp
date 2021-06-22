
#include "ChatServer.h"


using namespace std;
void ChatServer::sendToAll(string s, TcpServer::Client* sender) {
	cout << s << endl;
	for_each(clients.cbegin(), clients.cend(), [s, sender](std::pair<string, TcpServer::Client*> const& c) {
		if (c.second != sender)
			c.second->sendData(s.c_str(), s.length() + 1);
		});
}
void ChatServer::checkNewBees() {
	newBees.erase(std::remove_if(
		newBees.begin(),
		newBees.end(),
		[this](TcpServer::Client* client) {
			auto nReadBytes = client->loadData();
			if (nReadBytes == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					delete client;
					return true;
				}
			}
			if (nReadBytes > 0) {
				auto name = client->getData();
				clients.push_back(std::pair<string, TcpServer::Client*>(name, client));
				string welcome = "Welcome to chat ";
				sendToAll(welcome + name, NULL);
				return true;
			}
			return false;
		}), newBees.end());
}
void ChatServer::checkClients() {
	clients.erase(std::remove_if(
		clients.begin(),
		clients.end(),
		[this](std::pair<string, TcpServer::Client*> pair) {
			auto name = pair.first;
			auto client = pair.second;
			auto nReadBytes = client->loadData();
			if (nReadBytes == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					sendToAll(name + " leave chat ", client);
					delete client;
					return true;
				}
			}
			if (nReadBytes > 0) {
				auto text = client->getData();
				sendToAll(name + " : " + text, client);
			}
			return false;

		}), clients.end());
}
void ChatServer::start(int port) {
	auto proccess = thread([this]() {
		while (true)
		{
			checkNewBees();
			checkClients();
		}
		});
	TcpServer server(port,
		[this](TcpServer::Client* client) {
			newBees.push_back(client);
		}
	);
	if (server.start() == TcpServer::status::up) {
		std::cout << "Server is up!" << std::endl;
		server.joinLoop();
	}
	else {
		std::cout << "Server start error! Error code:" << int(server.getStatus()) << std::endl;
		proccess.detach();
		return;
	}
	proccess.join();
}
