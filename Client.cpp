#include "Client.h"

int Client::clientSocketDescriptor = -1;
std::string Client::nickname = "";

void* Client::clientInput(void*) {
    char messageBuffer[250];
    while (1) {
        std::cout << ">";
        std::string clientMessage;
        std::string nicknameContainer = nickname + " : ";
        getline(std::cin, clientMessage);
        nicknameContainer += clientMessage;
        memset(&messageBuffer, 0, sizeof(messageBuffer));
        strcpy(messageBuffer, nicknameContainer.c_str());
        if (clientMessage == "exit") {
            send(clientSocketDescriptor, (char *) &messageBuffer, strlen(messageBuffer), 0);
            close(clientSocketDescriptor);
            exit(0);
        }
        send(clientSocketDescriptor, (char *) &messageBuffer, 250, 0);
        memset(&messageBuffer, 0, sizeof(messageBuffer));
    }
}

void* Client::clientOutput(void*) {
    char messageBuffer[250];
    while (1) {
        memset(&messageBuffer, 0, sizeof(messageBuffer));
        recv(clientSocketDescriptor, (char *) &messageBuffer, 250, 0);
        if (!strcmp(messageBuffer, "exit")) {
            close(clientSocketDescriptor);
            exit(0);
        }
        std::cout << messageBuffer << std::endl;
    }
}

Client::Client(std::string serverIp, int port) {
    struct hostent *host = gethostbyname(serverIp.c_str());
    bzero((char *) &client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr =
            inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
    client.sin_port = htons(port);
    Client::clientSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
}

void Client::connectToServer() {
    std::cout << "Enter nickname : ";
    std::cin >> nickname;
    int connectStatus = connect(clientSocketDescriptor, (sockaddr *) &client, sizeof(client));
    if (connectStatus < 0) {
        throw std::exception();
    }
    std::cout << "Connected to the server!" << std::endl;
    pthread_t inputThread, outputThread;
    pthread_create(&inputThread, NULL, &clientInput, (void *) clientSocketDescriptor);
    pthread_create(&outputThread, NULL, &clientOutput, (void *) clientSocketDescriptor);
    pthread_join(inputThread, 0);
}