#include <numeric>
#include <iostream>
#include <thread>

#include <boost/thread/barrier.hpp>

#include "multicounter.h"
#include "numa-multicounter.h"

uint64_t execute_for(const std::function<void()> & function, int ms = 1000, int subticks = 1000) {
    uint64_t num_calls = 0;
    auto start = std::chrono::steady_clock::now();
    while (true) {
        for (int i = 0; i < subticks; i++) {
            function();
            num_calls++;
        }
        auto end = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > ms) {
            break;
        }
    }
    return num_calls;
}

void pin_thread(std::size_t thread_id, std::thread & thread) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id, &cpuset);
    int rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    (void)rc;
}

void ops_thread_routine(boost::barrier & barrier, multicounter & m, uint64_t & num_ops) {
    barrier.wait();
    num_ops = 2 * execute_for([&m]() {
        m.add();
        m.get();
    });
}

int bench_multicounter(std::size_t num_threads, std::size_t num_counters) {
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_ops_counters(num_threads);
    multicounter m(num_counters, 0);
    boost::barrier barrier(num_threads);
    for (std::size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        threads.emplace_back(ops_thread_routine, std::ref(barrier), std::ref(m),
                             std::ref(num_ops_counters[thread_id]));
        pin_thread(thread_id, threads.back());
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    int mops = std::accumulate(num_ops_counters.begin(), num_ops_counters.end(), 0ULL) / (int)1e6;
    return mops;
}

void numa_ops_thread_routine(int node_id, boost::barrier & barrier, numa_multicounter & m, uint64_t & num_ops) {
    barrier.wait();
    num_ops = 2 * execute_for([node_id, &m]() {
        m.add(node_id);
        m.get(node_id);
    });
}

int bench_numa_multicounter(std::size_t num_threads, std::size_t num_counters_on_each_node) {
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_ops_counters(num_threads);
    numa_multicounter m(num_counters_on_each_node);
    boost::barrier barrier(num_threads);
    for (std::size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        threads.emplace_back(numa_ops_thread_routine, thread_id / 18, std::ref(barrier), std::ref(m),
                             std::ref(num_ops_counters[thread_id]));
        pin_thread(thread_id, threads.back());
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    int mops = std::accumulate(num_ops_counters.begin(), num_ops_counters.end(), 0ULL) / (int)1e6;
    return mops;
}

int main() {
    int mops;
    mops = bench_multicounter(18, 1);
    std::cout << mops << std::endl;
    mops = bench_numa_multicounter(18, 1);
    std::cout << mops << std::endl;
    return 0;
}