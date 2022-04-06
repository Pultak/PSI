project(task2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(task2 main.cpp HttpComm.cpp HttpComm.h Synchronization.cpp Synchronization.h EndPoints.cpp EndPoints.h)
