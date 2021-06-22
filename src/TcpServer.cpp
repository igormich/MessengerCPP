#include "TcpServer.h"
#include <chrono>

//Êîíñòðóêòîð ïðèíèìàåò:
//port - ïîðò íà êîòîðîì áóäåì çàïóñêàòü ñåðâåð
//handler - callback-ôóíêöèÿ çàïóñêàÿìàÿ ïðè ïîäêëþ÷åíèè êëèåíòà
//          îáúåêò êîòîðîãî è ïåðåäàþò ïåðâûì àðãóìåíòîì â callback
//          (ïðèìåð ëÿìáäà-ôóíêöèè: [](TcpServer::Client){...do something...})
TcpServer::TcpServer(const uint16_t port, handler_function_t handler) : port(port), handler(handler), w_data(WSAData()) {}

//Äåñòðóêòîð îñòàíàâëèâàåò ñåðâåð åñëè îí áûë çàïóùåí
//è âû÷èùàåò çàäàííóþ âåðñèþ WinSocket
TcpServer::~TcpServer() {
    if (_status == status::up)
        stop();
#ifdef _WIN32 // Windows NT
    WSACleanup();
#endif
}

//Çàäà¸ò callback-ôóíêöèþ çàïóñêàÿìóþ ïðè ïîäêëþ÷åíèè êëèåíòà
void TcpServer::setHandler(TcpServer::handler_function_t handler) { this->handler = handler; }

//Getter/Setter ïîðòà
uint16_t TcpServer::getPort() const { return port; }
uint16_t TcpServer::setPort(const uint16_t port) {
    this->port = port;
    restart(); //Ïåðåçàïóñòèòü åñëè ñåðâåð áûë çàïóùåí
    return port;
}

//Ïåðåçàïóñê ñåðâåðà
TcpServer::status TcpServer::restart() {
    if (_status == status::up)
        stop();
    return start();
}

// Âõîä â ïîòîê îáðàáîòêè ñîåäèíåíèé
void TcpServer::joinLoop() { handler_thread.join(); }

//Çàãðóæàåò â áóôåð äàííûå îò êëèåíòà è âîçâðàùàåò èõ ðàçìåð
int TcpServer::Client::loadData() { return recv(socket, buffer, buffer_size, 0); }
//Âîçâðàùàåò óêàçàòåëü íà áóôåð ñ äàííûìè îò êëèåíòà
char* TcpServer::Client::getData() { return buffer; }
//Îòïðàâëÿåò äàííûå êëèåíòó
bool TcpServer::Client::sendData(const char* buffer, const size_t size) const {
    if (send(socket, buffer, size, 0) < 0) return false;
    return true;
}

#ifdef _WIN32 // Windows NT
//Çàïóñê ñåðâåðà
TcpServer::status TcpServer::start() {
    WSAStartup(MAKEWORD(2, 2), &w_data); //Çàäà¸ì âåðñèþ WinSocket

    SOCKADDR_IN address; //Ñòðóêòóðà õîñò/ïîðò/ïðîòîêîë äëÿ èíèöèàëèçàöèè ñîêåòà
    address.sin_addr.S_un.S_addr = INADDR_ANY; //Ëþáîé IP àäðåññ
    address.sin_port = htons(port); //Çàäà¸ì ïîðò
    address.sin_family = AF_INET; //AF_INET - Cåìåéñòâî àäðåñîâ äëÿ IPv4

    //Èíèöèàëèçèðóåì íàø ñîêåò è ïðîâåðÿåì êîððåêòíî ëè ïðîøëà èíèöèàëèçàöèÿ
    //â ïðîòèâíîì ñëó÷àå âîçâðàùàåì ñòàòóñ ñ îøèáêîé
    if (static_cast<int>(serv_socket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) return _status = status::err_socket_init;

    //Ïðèñâàèâàåì ê ñîêåòó àäðåññ è ïîðò è ïðîâåðÿåì íà êîðåêòíîñòü ñîêåò
    //â ïðîòèâíîì ñëó÷àå âîçâðàùàåì ñòàòóñ ñ îøèáêîé
    if (bind(serv_socket, (struct sockaddr*) & address, sizeof(address)) == SOCKET_ERROR) return _status = status::err_socket_bind;
    //Çàïóñêàåì ïðîñëóøêó è ïðîâåðÿåì çàïóñòèëàñü ëè îíà
    //â ïðîòèâíîì ñëó÷àå âîçâðàùàåì ñòàòóñ ñ îøèáêîé
    if (listen(serv_socket, SOMAXCONN) == SOCKET_ERROR) return _status = status::err_socket_listening;

    //Ìåíÿåì ñòàòóñ, çàïóñêàåì îáðàáîò÷èê ñîåäèíåíèé è âîçâðàùàåì ñòàòóñ
    _status = status::up;
    handler_thread = std::thread([this] {handlingLoop(); });
    return _status;
}

//Îñòàíîâêà ñåðâåðà
void TcpServer::stop() {
    _status = status::close; //Èçìåíåíèå ñòàòóñà
    closesocket(serv_socket); //Çàêðûòèå ñîêåòà
    joinLoop(); //Îæèäàíèå çàâåðøåíèÿ
    for (std::thread& cl_thr : client_handler_threads) //Ïåðåáîð âñåõ êëèåíòñêèõ ïîòîêîâ
        cl_thr.join(); // Îæèäàíèå èõ çàâåðøåíèÿ
    client_handler_threads.clear(); // Î÷èñòêà ñïèñêà êëèåíòñêèõ ïîòîêîâ
    client_handling_end.clear(); // Î÷èñòêà ñïèñêà èäåíòèôèêàòîðîâ çàâåðø¸ííûõ êëèåíòñêèõ ïîòîêîâ
}

// Ôóíêèöÿ îáðàáîòêè ñîåäèíåíèé
void TcpServer::handlingLoop() {
    while (_status == status::up) {
        SOCKET client_socket; //Ñîêåò êëèåíòà
        SOCKADDR_IN client_addr; //Àäðåññ êëèåíòà
        int addrlen = sizeof(client_addr); //Ðàçìåð àäðåñà êëèåíòà
        //Ïîëó÷åíèå ñîêåòà è àäðåñà êëèåíòà
        //(åñëè ñîêåò êîðåêòåí è ñåðâåð çàðóùåí çàïóñê ïîòîêà îáðàáîòêè)
        if ((client_socket = accept(serv_socket, (struct sockaddr*) & client_addr, &addrlen)) != 0 && _status == status::up) {
            unsigned long ul = 1;
            ioctlsocket(client_socket, FIONBIO, (unsigned long*)&ul);
            client_handler_threads.push_back(std::thread([this, &client_socket, &client_addr] {
                handler(new Client(client_socket, client_addr)); //Çàïóñê callback-îáðàáîò÷èêà
                //Äîáàâëåíèå èäåíòèôèêàòîðà â ñïèñîê èäåíòèôèêàòîðîâ çàâåðø¸ííûõ êëèåíòñêèõ ïîòîêîâ
                client_handling_end.push_back(std::this_thread::get_id());
                }));
        }
        //Î÷èñòêà îòðàáîòàííûõ êëèåíòñêèõ ïîòîêîâ
        if (!client_handling_end.empty())
            for (std::list<std::thread::id>::iterator id_it = client_handling_end.begin(); !client_handling_end.empty(); id_it = client_handling_end.begin())
                for (std::list<std::thread>::iterator thr_it = client_handler_threads.begin(); thr_it != client_handler_threads.end(); ++thr_it)
                    if (thr_it->get_id() == *id_it) {
                        thr_it->join();
                        client_handler_threads.erase(thr_it);
                        client_handling_end.erase(id_it);
                        break;
                    }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Êîíñòðóêòîð êëèåíòà ïî ñîêåòó è àäðåñó
TcpServer::Client::Client(SOCKET socket, SOCKADDR_IN address) : socket(socket), address(address) {}
// Êîíñòðóêòîð êîïèðîâàíèÿ
TcpServer::Client::Client(const TcpServer::Client& other) : socket(other.socket), address(other.address) {}

TcpServer::Client::~Client() {
    shutdown(socket, 0); //Îáðûâ ñîåäèíåíèÿ ñîêåòà
    closesocket(socket); //Çàêðûòèå ñîêåòà
}

// Ãåòòåðû õîñòà è ïîðòà
uint32_t TcpServer::Client::getHost() const { return address.sin_addr.S_un.S_addr; }
uint16_t TcpServer::Client::getPort() const { return address.sin_port; }

#else // *nix

//Çàïóñê ñåðâåðà (ïî àíàëîãèè ñ ðåàëèçàöèåé äëÿ Windows)
TcpServer::status TcpServer::start() {
    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    serv_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (serv_socket == -1) return _status = status::err_socket_init;
    if (bind(serv_socket, (struct sockaddr*) & server, sizeof(server)) < 0) return _status = status::err_socket_bind;
    if (listen(serv_socket, 3) < 0)return _status = status::err_socket_listening;

    _status = status::up;
    handler_thread = std::thread([this] {handlingLoop(); });
    return _status;
}

//Îñòàíîâêà ñåðâåðà
void TcpServer::stop() {
    _status = status::close;
    close(serv_socket);
    joinLoop();
    for (std::thread& cl_thr : client_handler_threads)
        cl_thr.join();
    client_handler_threads.clear();
    client_handling_end.clear();
}

// Ôóíêèöÿ îáðàáîòêè ñîåäèíåíèé (ïî àíàëîãèè ñ ðåàëèçàöèåé äëÿ Windows)
void TcpServer::handlingLoop() {
    while (_status == status::up) {
        int client_socket;
        struct sockaddr_in client_addr;
        int addrlen = sizeof(struct sockaddr_in);
        if ((client_socket = accept(serv_socket, (struct sockaddr*) & client_addr, (socklen_t*)&addrlen)) >= 0 && _status == status::up)
            client_handler_threads.push_back(std::thread([this, &client_socket, &client_addr] {
            handler(Client(client_socket, client_addr));
            client_handling_end.push_back(std::this_thread::get_id());
                }));

        if (!client_handling_end.empty())
            for (std::list<std::thread::id>::iterator id_it = client_handling_end.begin(); !client_handling_end.empty(); id_it = client_handling_end.begin())
                for (std::list<std::thread>::iterator thr_it = client_handler_threads.begin(); thr_it != client_handler_threads.end(); ++thr_it)
                    if (thr_it->get_id() == *id_it) {
                        thr_it->join();
                        client_handler_threads.erase(thr_it);
                        client_handling_end.erase(id_it);
                        break;
                    }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Êîíñòðóêòîð êëèåíòà ïî ñîêåòó è àäðåñó
TcpServer::Client::Client(int socket, struct sockaddr_in address) : socket(socket), address(address) {}
// Êîíñòðóêòîð êîïèðîâàíèÿ
TcpServer::Client::Client(const TcpServer::Client& other) : socket(other.socket), address(other.address) {}

TcpServer::Client::~Client() {
    shutdown(socket, 0); //Îáðûâ ñîåäèíåíèÿ ñîêåòà
    close(socket); //Çàêðûòèå ñîêåòà
}

// Ãåòòåðû õîñòà è ïîðòà
uint32_t TcpServer::Client::getHost() { return address.sin_addr.s_addr; }
uint16_t TcpServer::Client::getPort() { return address.sin_port; }

#endif
