#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netdb.h>

class Client {
private :
    static int clientSocketDescriptor;
    static std::string nickname;
    sockaddr_in client;

    static void* clientInput(void*);
    static void* clientOutput(void*);

public:
    Client(std::string serverIp, int port);
    void connectToServer();
};