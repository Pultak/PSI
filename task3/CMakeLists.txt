cmake_minimum_required(VERSION 3.0)
project(task3)

set(CMAKE_CXX_STANDARD 17)


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/")

add_executable(task3 main.cpp apifetcher.cpp apifetcher.h)
