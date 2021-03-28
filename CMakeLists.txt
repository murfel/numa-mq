cmake_minimum_required(VERSION 3.7.2)
project(numa_mq)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Werror -pedantic -pthread -lnuma")

set(BOOST_INCLUDEDIR "/nfs/scistore14/alistgrp/vaksenov/murfel/multiqueue/boost_1_75_0")

find_package(Boost COMPONENTS thread REQUIRED)

add_executable(main main.cpp counters/abstract_counter.h counters/hi_thru.h counters/hi_acc.h
        counters/two_choice.h counters/two_choice_avg.h counters/two_choice_avg_plus_delta.h
        numa_counters/numa_hi_thru.h numa_counters/numa_hi_thru_add_hi_acc_get.h)
target_link_libraries(main PRIVATE Boost::thread)

add_executable(pin_thread_example pin_thread_example.cpp)
target_link_libraries(pin_thread_example PRIVATE Boost::thread)
