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
    /**
     * This function adds a new job to the queue for the worker threads
     * @param userSocket socket that the user is connected on
     * @param userIp ip of the connected user
     */
    void addNewWork(int userSocket, char* userIp);

    /**
     * This function returns and pops the first job from the job queue
     * @param work pair that will be over writen by the popped value
     */
    bool getWork(std::pair<int, std::string>& work);

private:
    /**
     * Queue of all newly received requests
     */
    std::queue<std::pair<int, std::string>> workQueue{};

    /**
     * Synchronization primitive for the queue access
     */
    std::mutex connectionMutex{};


public:
    /**
     * This function synchronizes with all the worker threads and terminates them
     */
    void stopAndWaitWorkers();

private:

    /**
     * Flag used to identify if the application should be terminated soon or otherwise
     */
    bool running = true;


private:

    /**
     * Function map representing all the possible end points
     */
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

    /**
     * Function extracts the
     * @param result result string that will be over written by the collected end point resource
     * @param buffer message buffer received from user
     */
    static void extractEndPointResource(std::string& result, const std::string& buffer);

    /**
     * Function that checks if the received HTTP GET request is valid
     * @param buffer message buffer received from user
     * @return true => if ok, else not ok
     */
    static bool validateHTTPRequest(const std::string& buffer);
    /**
     * Function reads the user request and if valid sends to the user data from the desired end point
     * @param userSocket socket that the user is connected on
     * @param userIp ip of the connected user
     */
    void processUserRequest(int userSocket, const std::string& ip);

    /**
     * Entry point of the newly created thread.
     * Function contains infinite loop in which the worker threads wait for new request jobs.
     * After assignment of new job the thread checks for user message until defined timeout.
     */
    void workerEntryPoint();
};

#endif //TASK2_HTTPCOMM_H
