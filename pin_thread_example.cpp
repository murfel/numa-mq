#include <vector>
#include <thread>
#include <sched.h>
#include <numa.h>
#include <iostream>
#include <atomic>

#include <boost/thread/barrier.hpp>


uint64_t execute_for_ops(const std::function<void()> & function, std::size_t ops) {
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < ops; i++) {
        function();
    }
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

std::atomic<int> m{0};
void thread_routine(boost::barrier & barrier, int thread_id) {
    barrier.wait();
//    assert(sched_getcpu() == thread_id);
//    assert(sched_getcpu() / 18 == numa_node_of_cpu(sched_getcpu()));
    int s;
    auto r = execute_for_ops(
            [&s]() { for (int i = 0; i < 100; i++) { s += i; } },
            1'000'000'0000);
    while (m != thread_id) { continue; }
    std::cout << thread_id << " " << r << std::endl;
    m++;
}

void pin_thread(int cpu, std::thread & thread) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    (void)rc;
}

int main() {
    std::size_t num_threads = 4 * 18;
    std::vector<std::thread> threads;
    boost::barrier barrier(num_threads + 1);
    for (std::size_t i = 0; i < num_threads; i++) {
        threads.emplace_back(thread_routine, std::ref(barrier), i);
        pin_thread(i, threads.back());
    }
    barrier.wait();
    for (auto & thread: threads) {
        thread.join();
    }
    return 0;
}