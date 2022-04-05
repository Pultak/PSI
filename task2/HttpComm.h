//
// Created by pultak on 05.04.22.
//

#ifndef TASK2_HTTPCOMM_H
#define TASK2_HTTPCOMM_H


#include <condition_variable>
#include <map>
#include "Synchronization.h"
#include "EndPoints.h"

#define SOCKET_TIMEOUT_SEC 10
#define RECEIVE_BUFFER_SIZE 1024

class HttpComm {

public:
    HttpComm();

public:


public:
    void addNewWork(int userSocket, char* userIp);
    bool getWork(std::pair<int, std::string>& work);
    void removeWork();



    void stopAndWaitWorkers();

private:

    std::unique_ptr<Synchronization::Semaphore> workerSemaphore = std::make_unique<Synchronization::Semaphore>();

private:
    int prepareWorkerThreads();

    std::map<std::string, void(*)(std::string&)> endPointMap = {
            {std::string("/hello-world/"), EndPoints::helloWorld},
            {std::string("/end-points/"), EndPoints::getAllEndPoints},
            {std::string("/info/"), EndPoints::getInfo},
            {std::string("/xxx/"), EndPoints::getTopSecret}
    };


private:
    bool extractEndPointResource(const std::string& buffer);
    bool validateHTTPRequest(const std::string& buffer);
    void processUserRequest(int userSocket, const std::string& ip);

    void workerEntryPoint();



};


#endif //TASK2_HTTPCOMM_H
