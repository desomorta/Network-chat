#include "Client.h"

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cerr << "Usage: ip_address port" << std::endl;
        exit(1);
    }
    Client client(argv[1],atoi(argv[2]));
    try {
        client.connectToServer();
    } catch (std::exception) {
        std::cout << "Error connecting to server" << std::endl;
    }
    return 0;
}