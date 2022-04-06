#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#include <unistd.h>
#include "HttpComm.h"


#define SERVER_ACCEPT_DELAY_MS 10000

struct ConnectionInfo{
    struct sockaddr_in deviceAddress {}; // used for receiving messages from devices
    int socket{}; //server socket file descriptor
    int port{}; //port we are listening on
    char* inetAddress{}; //address of the server
};


int initServer(ConnectionInfo &config){
    std::cout << "Server init start!" << std::endl;

    //define new socket file descriptor
    if ((config.socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Definition of socket file descriptor failed!" << std::endl;
        exit(1);
    }

    int opt = 1;
    if (setsockopt(config.socket, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        std::cerr << "Cant attach the socket to the defined port!" << std::endl;
        exit(1);
    }

    config.deviceAddress.sin_family = AF_INET;
    config.deviceAddress.sin_addr.s_addr = inet_addr(config.inetAddress);
    config.deviceAddress.sin_port = htons(config.port);

    signal(SIGPIPE, SIG_IGN);
    if (bind(config.socket, reinterpret_cast<struct sockaddr *>(&config.deviceAddress), sizeof(config.deviceAddress)) < 0) {
        std::cerr << "Bind failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(config.socket, 5) < 0) {
        std::cerr << "listening failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server init done!" << std::endl;
    return 0;
}



int startAccepting(const ConnectionInfo& config, HttpComm& comm){
    int userSocket; //socket for the connected user
    std::string connectedIp; //ip of the connected user
    int addressLen = sizeof(config.deviceAddress);

    struct sockaddr_in newAddr{};
    auto socketLenPtr = reinterpret_cast<socklen_t *>(&addressLen);

    std::cout << "Starting accepting requests!" << std::endl;
    while (true) {
        if ((userSocket = accept(config.socket, reinterpret_cast<sockaddr *>(&newAddr), socketLenPtr)) < 0) {
            std::cout << "Error occurred during accept!" << std::endl;
            comm.stopAndWaitWorkers();
            return 1;
        }

        //new connection started => assign work for the worker threads
        char *ip = inet_ntoa(newAddr.sin_addr);
        comm.addNewWork(userSocket, ip);

        std::cout << "New connection with " << ip << " was established!" << std::endl;
        usleep(SERVER_ACCEPT_DELAY_MS);
    }
}


int main(int argc, char* argv[]) {
    if(argc < 3){
        std::cout << "Not enough parameters passed!" << std::endl;
        std::cout << "Usage: ./task1 <inet-address> <timeout>" << std::endl;
        return 1;
    }

    char* inetAddr = argv[1];
    int port = std::stoi(argv[2]);

    if(port <= 0){
        std::cout << "Please enter valid timeout!" << std::endl;
        return 1;
    }

    struct ConnectionInfo config {};
    config.inetAddress = inetAddr;
    config.port = port;
    initServer(config);

    HttpComm httpCommunication{};

    return startAccepting(config, httpCommunication);
}
