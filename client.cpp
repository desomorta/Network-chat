#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <ncurses.h>

WINDOW *inpScreen;
WINDOW *outScreen;
WINDOW *mainScreen;

//wprintw(window,message); - print message
//wrefresh(window) - update window
//wmove(window,x,y) - moves
//wcursyncup(window) - show which window will be active for input rn
//noecho() - input w/o print
//scrollok(window,true/false) - for scrolling
//cbreak() - input POSIMVILNY
//subwin(fath. window,height,weight,y,x) - create subwindow
//initscr() - initialize ncurses and back fathers window
//endwin() - deinitialize ncurses
//clear() - clear all windows
//refresh() - update all windows

class Client {
private :
    static int clientSocketDescriptor;
    static std::string nickname;
    sockaddr_in client;

    static void* clientInput(void*) {
        char messageBuffer[250];
        while (1) {
            std::string nicknameContainer = nickname + " : ";
            std::string clientMessage = getUserInput();
            if (clientMessage.empty()) {
                continue;
            }
            nicknameContainer += clientMessage;
            memset(&messageBuffer, 0, sizeof(messageBuffer));
            strcpy(messageBuffer, nicknameContainer.c_str());
            send(clientSocketDescriptor, (char *) &messageBuffer, nicknameContainer.length(), 0);
            if (clientMessage == "exit") {
                close(clientSocketDescriptor);
                endwin();
                exit(0);
            }
        }
    }

    static bool isAllowedSymbol(int symbol) {
        return symbol >= 0x20 && symbol <= 0x2F || symbol >= 0x3A && symbol <= 0x40 || symbol >= 0x5B && symbol <= 0x60 ||
        symbol >= 0x7B && symbol <= 0x7E;
    }
    static bool isAlphaEN(int symbol) {
        return symbol >= 0x41 && symbol <= 0x5A || symbol >= 0x61 && symbol <= 0x7A;
    }

    static std::string getUserInput()
    {
        std::string input;

        wmove(inpScreen, 0, 0);
        wrefresh(inpScreen);
        wprintw(inpScreen, "> ");

        int symbol;

        while((symbol = getch()) != '\n') {
            if (symbol == 0x8 || symbol == 0x79)
                if (!input.empty()) {
                    wprintw(inpScreen, "\b \b\0");
                    input.pop_back();
                    wrefresh(inpScreen);
                } else
                    wprintw(inpScreen, "\b \0");
            else if (isAllowedSymbol(symbol) || isAlphaEN(symbol)) {
                input.push_back(symbol);
                wprintw(inpScreen, (char *) &symbol);
                wrefresh(inpScreen);
            }
        }

        wclear(inpScreen);
        wrefresh(inpScreen);

        return input;
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
        int dataStart = getMessageStart(std::string(message));
        if(dataStart == -1)
        {
            return std::string();
        }
        return std::string(message + dataStart);
    }

    static void* clientOutput(void*) {
        char messageBuffer[250];
        while (1) {
            memset(&messageBuffer, 0, sizeof(messageBuffer));
            recv(clientSocketDescriptor, (char *) &messageBuffer, 250, 0);
            if(parseMessage(messageBuffer) == "exit") {
                close(clientSocketDescriptor);
                endwin();
                exit(0);
            }
            wprintw(outScreen,"%s\n",messageBuffer);
            wrefresh(outScreen);
            wcursyncup(inpScreen);
            wrefresh(inpScreen);
        }
    }

public:
    Client(std::string serverIp, int port) {
        struct hostent *host = gethostbyname(serverIp.c_str());
        bzero((char *) &client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_addr.s_addr =
                inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
        client.sin_port = htons(port);
        Client::clientSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    }

    void connectToServer() {
        WINDOW *inpTableScreen;
        WINDOW *outTableScreen;
        std::cout << "Enter nickname : ";
        std::cin >> nickname;
        int connectStatus = connect(clientSocketDescriptor, (sockaddr *) &client, sizeof(client));
        if (connectStatus < 0) {
            throw std::exception();
        }
        mainScreen = initscr();
        outTableScreen = subwin(mainScreen,LINES*0.7,COLS,0,0);
        inpTableScreen = subwin(mainScreen,LINES*0.3,COLS,LINES*0.7,0);
        outScreen = subwin(mainScreen,LINES * 0.7 - 2,COLS - 2,1,1);
        inpScreen = subwin(mainScreen,LINES * 0.3 - 2,COLS - 2,LINES * 0.7 + 1,1);
        wborder(inpTableScreen,'|','|','-','-','|','|','|','|');
        wborder(outTableScreen,'|','|','-','-','|','|','|','|');
        scrollok(outScreen,TRUE);
        wcursyncup(inpScreen);
        noecho();
        cbreak();
        refresh();
        pthread_t inputThread, outputThread;
        pthread_create(&inputThread, NULL, &clientInput,NULL);
        pthread_create(&outputThread, NULL, &clientOutput,NULL);
        pthread_join(inputThread, 0);
    }
};


int Client::clientSocketDescriptor = -1;
std::string Client::nickname = "";

int main(int argc, char *argv[]) {
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