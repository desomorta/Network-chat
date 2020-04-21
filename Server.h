#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <vector>

class Server {
private:
    static std::vector<int> clients;
    static int serverSocketDescriptor;
    sockaddr_in server;

    static void* serverInput(void*);
    static void* connectToClient(void*);
    void waitForConnection();

public:
    Server(int port);
    void waitForClients();
};