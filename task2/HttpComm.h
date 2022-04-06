//
// Created by pultak on 05.04.22.
//

#ifndef TASK2_HTTPCOMM_H
#define TASK2_HTTPCOMM_H


#include <condition_variable>
#include <map>
#include <list>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include "Synchronization.h"
#include "EndPoints.h"

#define SOCKET_TIMEOUT_SEC 10
#define RECEIVE_BUFFER_SIZE 1024

#define WORKER_COUNT 20

class HttpComm {

public:
    HttpComm();
    ~HttpComm() = default;
public:
    void addNewWork(int userSocket, char* userIp);
    bool getWork(std::pair<int, std::string>& work);
    void stopAndWaitWorkers();

private:

    bool running = true;
    std::mutex connectionMutex{};

    std::queue<std::pair<int, std::string>> workQueue{};

private:
    std::map<std::string, void(*)(std::string&)> endPointMap = {
            {std::string("/"), EndPoints::getAllEndPoints},
            {std::string("/hello-world/"), EndPoints::helloWorld},
            {std::string("/hello-world"), EndPoints::helloWorld},
            {std::string("/end-points/"), EndPoints::getAllEndPoints},
            {std::string("/end-points"), EndPoints::getAllEndPoints},
            {std::string("/info/"), EndPoints::getInfo},
            {std::string("/info"), EndPoints::getInfo},
            {std::string("/xxx/"), EndPoints::getTopSecret},
            {std::string("/xxx"), EndPoints::getTopSecret}
    };


private:
    static void extractEndPointResource(std::string& result, const std::string& buffer);
    static bool validateHTTPRequest(const std::string& buffer);
    void processUserRequest(int userSocket, const std::string& ip);

    void workerEntryPoint();



};


#endif //TASK2_HTTPCOMM_H
