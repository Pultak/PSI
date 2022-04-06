//
// Created by pultak on 05.04.22.
//

#ifndef TASK2_SYNCHRONIZATION_H
#define TASK2_SYNCHRONIZATION_H


#include <mutex>
#include <condition_variable>

namespace Synchronization {
    class Semaphore {
    public:
        explicit Semaphore(int threadCount): threadCount(threadCount) {
            count = 0;
        }

        inline void notify(){
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        inline void notifyAll(){
            std::unique_lock<std::mutex> lock(mtx);
            count += threadCount;
            cv.notify_all();
        }

        inline void wait(){
            std::unique_lock<std::mutex> lock(mtx);
            while (count == 0) {
                cv.wait(lock);
            }
            count--;
        }

        std::mutex mtx;
    private:
        std::condition_variable cv;
        size_t count;
        int threadCount;
    };
}
#endif //TASK2_SYNCHRONIZATION_H
