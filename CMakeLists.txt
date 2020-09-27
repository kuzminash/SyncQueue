cmake_minimum_required(VERSION 3.15)
project(lab_17)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(lab_17 src/main.cpp include/sync_queue.h include/helper.h)
target_link_libraries(lab_17 Threads::Threads)