cmake_minimum_required(VERSION 3.7.2)
project(numa_mq)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Werror -pedantic -pthread -lnuma")

find_package(Boost COMPONENTS thread REQUIRED)

add_executable(main main.cpp multicounter.h numa-multicounter.h)
target_link_libraries(main PRIVATE Boost::thread)
