//
// Created by pultak on 07.04.22.
//

#include "apifetcher.h"

#define BUFF_SIZE 1024


bool ApiFetcher::fetchData(std::string& result) {
    if(!connectToApi())
        return false;

    return sendAndReadRequest(result);
}

bool ApiFetcher::connectToApi() {
    char buffer[BUFF_SIZE];
    std::string response;

    // New socket file descriptor for specified API server
    if ((connectionInfo->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Failed to init socket file descriptor for: " << connectionInfo->inetAddress << ":" << connectionInfo->port << std::endl;
        return false;
    }

    // Initialize connection.
    connectionInfo->deviceAddress.sin_family = AF_INET;
    connectionInfo->deviceAddress.sin_port = htons(connectionInfo->port);
    if (inet_pton(AF_INET, connectionInfo->inetAddress, &(connectionInfo->deviceAddress.sin_addr)) <=0) {
        std::cout << "ERR: could not initialize connection\n";
        return "";
    }

    // Try to connect to specified server
    if (connect(connectionInfo->socket, reinterpret_cast<struct sockaddr *>(&connectionInfo->deviceAddress), sizeof(connectionInfo->deviceAddress)) < 0) {
        switch(errno){
            case EADDRNOTAVAIL:
                std::cout << "Address " << connectionInfo->inetAddress << " is not available from the local machine";
                break;
            case ECONNREFUSED:
                std::cout << "Address " << connectionInfo->inetAddress << " refused connection request";
                break;
            case ETIMEDOUT:
                std::cout << "Cant connect to " << connectionInfo->inetAddress << " before timeout!";
                break;
                //other errors can be found on https://pubs.opengroup.org/onlinepubs/009695399/functions/connect.html
            default:
                std::cout << "Cannot connect to " << connectionInfo->inetAddress << " because of non specified reasons.";
                break;
        }
        return false;
    }
    return true;
}

bool ApiFetcher::sendAndReadRequest(std::string& result) {
    char buffer[BUFF_SIZE];

    // Send our HTTP GET request to the server
    if (send(connectionInfo->socket, requestBody.c_str(), requestBody.length(), 0) == -1) {
        std::cout << "Failed to send data to server!\n";
        exit(EXIT_FAILURE);
    }

    // Read response from specified server
    size_t readBytes = read(connectionInfo->socket, buffer, BUFF_SIZE - 1);
    buffer[readBytes] = '\0';

    close(connectionInfo->socket);

    // Get the body part of the response
    result = std::string(buffer);
    std::size_t position = result.find("\r\n\r\n");
    if (position == std::string::npos) {
        std::cout << "Non valid data was received from server!\n";
        exit(EXIT_FAILURE);
    }

    result = result.substr(position - 1 + sizeof("\r\n\r\n"));
    //now trim JSON data to remove unwanted bloat;
    while (!result.empty() && result[0] != '{') {
        result = result.erase(0, 1);
    }
    while (!result.empty() && result[result.length() - 1] != '}') {
        result.pop_back();
    }

    return true;
}

ApiFetcher::ApiFetcher(int port, const char inetAddr[], std::string& requestBody) {
    this->connectionInfo->port = port;
    this->connectionInfo->inetAddress = inetAddr;
    this->requestBody = requestBody;
}
