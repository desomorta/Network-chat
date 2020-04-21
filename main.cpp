#include "Server.h"

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