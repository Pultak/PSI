//
// Created by pultak on 05.04.22.
//

#include "HttpComm.h"

std::unique_ptr<Synchronization::Semaphore> workerSemaphore = std::make_unique<Synchronization::Semaphore>(WORKER_COUNT);

std::list<std::thread*> workers{};

void HttpComm::workerEntryPoint() {
    std::pair<int, std::string> newWork{};

    while (running) {
        auto ok = this->getWork(newWork);
        if(ok){
            auto [userSocket, ip] = newWork;
            fd_set sockets;
            FD_ZERO(&sockets);
            struct timeval timeout{};
            auto begin = std::chrono::steady_clock::now();
            auto actual = begin;

            //while the time is in the timeout scope => select from user
            while (std::chrono::duration_cast<std::chrono::seconds>(actual - begin).count() < SOCKET_TIMEOUT_SEC) {
                FD_SET(userSocket, &sockets);
                timeout.tv_sec  = 0;
                timeout.tv_usec = SOCKET_TIMEOUT_SEC;
                select(FD_SETSIZE, &sockets, nullptr, nullptr, &timeout);

                // Check for data from the client socket.
                if (FD_ISSET(userSocket, &sockets)) {
                    processUserRequest(userSocket, ip);
                    break;
                }else{
                    actual = std::chrono::steady_clock::now();
                    continue;
                }
            }
            // Close up the client's connection.
            close(userSocket);
            std::cout << "client (" + ip + ") has been disconnected" << std::endl;
            break;
        }else{
            workerSemaphore->wait();
        }
    }
}

void HttpComm::processUserRequest(int userSocket, const std::string& ip) {
    char buffer[RECEIVE_BUFFER_SIZE];

    ssize_t receivedBytes = recv(userSocket, buffer, RECEIVE_BUFFER_SIZE - 1, 0);
    buffer[receivedBytes] = '\0';


    std::string bufferString = std::string(buffer);
    std::string resource{};
    std::string response{};

    // Check is it's a valid GET request.
    if (validateHTTPRequest(bufferString)) {
        extractEndPointResource(resource, bufferString);

        std::cout << "Received GET request from " + ip + " with resource: " + resource << std::endl;

        if(endPointMap.find(resource) != endPointMap.end()){
            std::string result{};
            endPointMap[resource](result);
            response = "HTTP/1.1 " + std::to_string(200) + "\n\n" + result;
        }else{
            std::cout << "Resource " + resource + " not found!"<< std::endl;

            response = "HTTP/1.1 " + std::to_string(404) + "\n\n";
        }

        std::cout << "Sending result to " + ip + " from resource: " + resource << std::endl;

        send(userSocket, response.c_str(), response.length(), 0);
    } else {
        std::cout << "Received INVALID GET request from  " + ip + "!" << std::endl;
    }
}

bool HttpComm::validateHTTPRequest(const std::string& buffer) {
    //starts with GET?
    if (buffer.rfind("GET ", 0) != 0) { // pos=0 limits the search to the prefix
        return false;
    }

    auto secondSpace = buffer.find(' ', 4);
    auto newLine = buffer.find('\n', 4);
    //is there a second white space? is there newline and is it after the second white space?
    if(secondSpace == -1 && newLine == -1 && secondSpace < newLine){
        return false;
    }
    auto httpVersion = buffer.substr(secondSpace + 1, newLine - secondSpace - 1);

    return std::equal(httpVersion.begin(), httpVersion.end(), "HTTP/1.1\r");
}

void HttpComm::stopAndWaitWorkers() {
    this->running = false;
    workerSemaphore->notifyAll();
    for (auto& thread : workers) {
        thread->join();
    }
}

void HttpComm::addNewWork(int userSocket, char *userIp) {
    const std::lock_guard<std::mutex> lock(this->connectionMutex);
    workQueue.push(std::pair(userSocket, userIp));
    workerSemaphore->notify();
}

bool HttpComm::getWork(std::pair<int, std::string> &work) {
    const std::lock_guard<std::mutex> lock(this->connectionMutex);
    if (this->workQueue.empty()) {
        return false;
    } else {
        work = this->workQueue.front();
        this->workQueue.pop();
        return true;
    }
}

HttpComm::HttpComm() {
    for (uint32_t i = 0; i < WORKER_COUNT; i++) {
        std::thread workerThread(&HttpComm::workerEntryPoint, this);
        workers.push_back(&workerThread);
        workerThread.detach();
    }
}

void HttpComm::extractEndPointResource(std::string& result, const std::string &buffer) {
    auto secondSpace = buffer.find(' ', 4);

    result = buffer.substr(4, secondSpace - 4);
}

