cmake_minimum_required(VERSION 3.7.2)
project(numa_mq)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic -pthread -lnuma")

set(BOOST_INCLUDEDIR "/nfs/scistore14/alistgrp/vaksenov/murfel/multiqueue/boost_1_75_0")

find_package(Boost COMPONENTS thread REQUIRED)

add_executable(main main.cpp counters/abstract_counter.h counters/dummy_high_throughput_counter.h counters/dummy_high_accuracy_counter.h
        counters/two_choice_counter.h numa_dummy_high_throughput_counter.h)
target_link_libraries(main PRIVATE Boost::thread)

add_executable(pin_thread_example pin_thread_example.cpp)
target_link_libraries(pin_thread_example PRIVATE Boost::thread)
