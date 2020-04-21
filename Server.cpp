#include "Server.h"

int Server::serverSocketDescriptor = -1;
std::vector<int> Server::clients = {};

void* Server::serverInput(void*) {
    std::string command;
    while (1) {
        std::cin.clear();
        std::cin >> command;
        if (command == "exit") {
            for (int i = 0; i < clients.size(); i++) {
                send(clients[i], "exit", 4, 0);
            }
            close(serverSocketDescriptor);
            exit(0);
        }
    }
}

void* Server::connectToClient(void*) {
    int currentClientSocketDescriptor = clients.at(clients.size() - 1);
    while (1) {
        char messageBuffer[250];
        memset(&messageBuffer, 0, 250);
        recv(currentClientSocketDescriptor, messageBuffer, 250, 0);
        if (!strcmp(messageBuffer, "exit")) {
            close(currentClientSocketDescriptor);
            break;
        }
        for (int i = 0; i < clients.size(); i++) {
            send(clients[i], messageBuffer, strlen(messageBuffer), 0);
        }
    }
    for (int i = 0; i < clients.size(); i++) {
        if (clients.at(i) == currentClientSocketDescriptor) {
            clients.erase(clients.begin() + i);
            break;
        }
    }
}

void Server::waitForConnection() {
    while (true) {
        sockaddr_in clientSocket;
        socklen_t clientSocketLength = sizeof(clientSocket);
        int clientSocketDescriptor = accept(serverSocketDescriptor, (sockaddr *) &clientSocket, &clientSocketLength);
        if (clientSocketDescriptor < 0) {
            std::cerr << "Error accepting request from client!" << std::endl;
            close(clientSocketDescriptor);
            continue;
        }
        pthread_t clientThread;
        clients.push_back(clientSocketDescriptor);
        std::cout << clientSocketDescriptor << std::endl;
        pthread_create(&clientThread, nullptr, &connectToClient, nullptr);
    }
}

Server::Server(int port) {
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    serverSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketDescriptor < 0) {
        throw std::exception();
    }
    int bindStatus = bind(serverSocketDescriptor, (struct sockaddr *) &server,
                          sizeof(server));
    if (bindStatus < 0) {
        throw std::exception();
    }
}

void Server::waitForClients() {
    pthread_t inputThread;
    pthread_create(&inputThread, NULL, &serverInput, NULL);
    listen(serverSocketDescriptor, 5);
    waitForConnection();
}