#include <vector>
#include <thread>
#include <mutex>
#include <sched.h>
#include <numa.h>

#include <boost/thread/barrier.hpp>

void thread_routine(boost::barrier & barrier) {
    barrier.wait();
    assert(sched_getcpu() / 18 == numa_node_of_cpu(sched_getcpu()));
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
    boost::barrier barrier(num_threads);
    for (std::size_t i = 0; i < num_threads; i++) {
        threads.emplace_back(thread_routine, std::ref(barrier));
        pin_thread(i, threads.back());
    }
    for (auto & thread: threads) {
        thread.join();
    }
    return 0;
}