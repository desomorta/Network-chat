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

    static void* serverInput(void*) {
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

    static int getMessageStart(std::string message) {
        for (size_t i = 0; i < message.length() - 2; i++)
        {
            if(message[i] == ':')
                return i + 2;
        }

        return -1;
    }

    static std::string parseMessage(char* message) {
        std::cout << "Parsing message from: " << message << std::endl;
        int dataStart = getMessageStart(std::string(message));
        if(dataStart == -1)
        {
            std::cout << "DataStart was zero\n";
            return std::string();
        }
        return std::string(message + dataStart);
    }

    static void* connectToClient(void*) {
        int currentClientSocketDescriptor = clients.at(clients.size() - 1);
        while (1) {
            char messageBuffer[250];
            memset(&messageBuffer, 0, 250);
            recv(currentClientSocketDescriptor, messageBuffer, 250, 0);
            std::string tempBuff = parseMessage(messageBuffer);
            if(tempBuff.empty()) {
                continue;
            }
            if (tempBuff == "exit") {
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

    void waitForConnection() {
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

public:
    Server(int port) {
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

    void waitForClients() {
        pthread_t inputThread;
        pthread_create(&inputThread, NULL, &serverInput, NULL);
        listen(serverSocketDescriptor, 5);
        waitForConnection();
    };
};

int Server::serverSocketDescriptor = -1;
std::vector<int> Server::clients = {};

int main(int argc, char** argv) {
    if(argc != 2)
    {
        std::cerr << "Usage: port" << std::endl;
        exit(1);
    }

    try {
        Server server(atoi(argv[1]));
        server.waitForClients();
    } catch (std::exception) {
        std::cout << "Error creating server" << std::endl;
    }
    return 0;
}