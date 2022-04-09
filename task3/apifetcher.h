//
// Created by pultak on 07.04.22.
//

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <memory>


#ifndef TASK3_APIFETCHER_H
#define TASK3_APIFETCHER_H


struct ConnectionInfo{
    struct sockaddr_in deviceAddress {}; // used for receiving messages from devices
    int socket{}; //server socket file descriptor
    int port{}; //port we are listening on
    const char* inetAddress{}; //address of the server
};


class ApiFetcher {
private:
    std::unique_ptr<ConnectionInfo> connectionInfo = std::make_unique<ConnectionInfo>();
    std::string requestBody;

public:
    ApiFetcher(int port, const char inetAddr[], std::string& requestBody);
    ~ApiFetcher() = default;

public:
    bool fetchData(std::string& result);

private:
    bool connectToApi();
    bool sendAndReadRequest(std::string& result);
};


#endif //TASK3_APIFETCHER_H
