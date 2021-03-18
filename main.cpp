#include <numeric>
#include <iostream>
#include <thread>

#include <boost/thread/barrier.hpp>

#include "multicounter.h"
#include "numa-multicounter.h"

/* Benchmarking utils */

struct multicounter_benchmark_results {
    std::vector<uint64_t> time_ms;
    std::vector<uint64_t> num_ops;
    std::vector<uint64_t> cnt_expected;
    std::vector<uint64_t> cnt_actual;
    explicit multicounter_benchmark_results(std::size_t num_threads) {
        time_ms.resize(num_threads);
        num_ops.resize(num_threads);
        cnt_expected.resize(num_threads);
        cnt_actual.resize(num_threads);
    }
};

std::size_t calc_num_nodes(std::size_t num_threads) {
    return num_threads / 18 + (num_threads % 18 > 0);
}

uint64_t abs_diff(uint64_t a, uint64_t b) {
    return a > b ? a - b : b - a;
}

double calc_mops_per_s(uint64_t num_ops, uint64_t ms) {
    // (num_ops / 1'000'000.0) / (ms / 1'000);
    return (double)num_ops / (double)ms / (double)1000.0;
}

uint64_t avg(std::vector<uint64_t> v) {
    return std::accumulate(v.begin(), v.end(), 0ULL) / v.size();
}

uint64_t execute_for_time(const std::function<void()> & function, int ms, int subticks = 1000) {
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

uint64_t execute_for_ops(const std::function<void()> & function, std::size_t ops) {
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < ops; i++) {
        function();
    }
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void pin_thread(std::size_t thread_id, std::thread & thread) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id, &cpuset);
    int rc = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    (void)rc;
}

/* Benchmarking functions */

void simple_multicounter_thread_routine_ops_for_time(int thread_id, boost::barrier & barrier, multicounter & m,
                                                     uint64_t & num_ops) {
    barrier.wait();
    num_ops = execute_for_time([thread_id, &m]() {
        m.add(thread_id);
    }, 1000);
}

uint64_t bench_simple_multicounter_ops_for_time(std::size_t num_threads, std::size_t num_counters) {
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_ops_counters(num_threads);
    multicounter m(num_counters, 0);
    boost::barrier barrier(num_threads);
    for (std::size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        threads.emplace_back(simple_multicounter_thread_routine_ops_for_time, thread_id, std::ref(barrier),
                             std::ref(m), std::ref(num_ops_counters[thread_id]));
        pin_thread(thread_id, threads.back());
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    uint64_t mops = std::accumulate(num_ops_counters.begin(), num_ops_counters.end(), 0ULL) / (uint64_t)1e6;
    return mops;
}

void numa_multicounter_thread_routine_ops_for_time(int node_id, int thread_id, boost::barrier & barrier,
                                                   numa_multicounter & m, uint64_t & num_ops) {
    barrier.wait();
    num_ops = execute_for_time([node_id, thread_id, &m]() {
        m.add(node_id, thread_id);
    }, 1000);
}

uint64_t bench_numa_multicounter_ops_for_time(std::size_t num_threads, std::size_t num_counters_on_each_node) {
    std::vector<std::thread> threads;
    std::vector<uint64_t> num_ops_counters(num_threads);
    const int num_nodes = calc_num_nodes(num_threads);
    numa_multicounter m(num_nodes, num_counters_on_each_node);
    boost::barrier barrier(num_threads);
    for (std::size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        threads.emplace_back(numa_multicounter_thread_routine_ops_for_time, thread_id / 18, thread_id,
                             std::ref(barrier), std::ref(m), std::ref(num_ops_counters[thread_id]));
        pin_thread(thread_id, threads.back());
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    uint64_t mops = std::accumulate(num_ops_counters.begin(), num_ops_counters.end(), 0ULL) / (uint64_t)1e6;
    return mops;
}

void numa_multicounter_thread_routine_time_for_ops(int node_id, int thread_id, boost::barrier & barrier,
        numa_multicounter & m, uint64_t num_ops, uint64_t & time_ms, uint64_t & cnt_actual) {
    barrier.wait();
    time_ms = execute_for_ops([node_id, thread_id, &m]() {
        m.add(node_id, thread_id);
    }, num_ops);
    barrier.wait();
    cnt_actual = m.get(node_id, thread_id);
}

multicounter_benchmark_results bench_numa_multicounter_time_for_ops(
        std::size_t num_threads, std::size_t num_counters_on_each_node, uint64_t num_ops = 1'000'000) {
    std::vector<std::thread> threads;
    const int num_nodes = calc_num_nodes(num_threads);
    numa_multicounter m(num_nodes, num_counters_on_each_node);
    boost::barrier barrier(num_threads);
    multicounter_benchmark_results bench_results(num_threads);
    std::fill(bench_results.num_ops.begin(), bench_results.num_ops.end(), num_ops);
    std::fill(bench_results.cnt_expected.begin(), bench_results.cnt_expected.end(), num_ops * num_threads);
    for (std::size_t thread_id = 0; thread_id < num_threads; thread_id++) {
        threads.emplace_back(numa_multicounter_thread_routine_time_for_ops, thread_id / 18, thread_id,
                std::ref(barrier), std::ref(m), bench_results.num_ops[thread_id],
                std::ref(bench_results.time_ms[thread_id]), std::ref(bench_results.cnt_actual[thread_id]));
        pin_thread(thread_id, threads.back());
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    return bench_results;
}

void bench_and_print_numa_multicounter_time_for_ops(
        std::size_t num_threads, std::size_t num_counters_on_each_node, uint64_t num_ops = 1'000'000) {
    auto res = bench_numa_multicounter_time_for_ops(num_threads, num_counters_on_each_node, num_ops);
    uint64_t time_ms = *std::max_element(res.time_ms.begin(), res.time_ms.end());

    std::vector<uint64_t> abs_diffs;
    abs_diffs.reserve(num_threads);
    for (std::size_t i = 0; i < res.cnt_expected.size(); i++) {
        abs_diffs.push_back(abs_diff(res.cnt_expected[i], res.cnt_actual[i]));
    }
    uint64_t avg_abs_diff = avg(abs_diffs);
    std::cout << num_threads << " threads (" << calc_num_nodes(num_threads) << " nodes), "
              << num_counters_on_each_node << " counters per node: ";
    std::cout << time_ms << " ms (" << (int)calc_mops_per_s(num_ops * num_threads, time_ms) << " mops/s), ";
    std::cout << avg_abs_diff << " avg diff"<< std::endl;
}

int main() {
    const int T = 18;  // num threads on one node

    for (int num_counter = 1; num_counter <= 18; num_counter++) {
        uint64_t mops = bench_simple_multicounter_ops_for_time(T, num_counter);
        std::cout << mops << " " << std::flush;
    } std::cout << std::endl;

    for (int mult = 1; mult <= 4; mult++) {
        uint64_t mops = bench_simple_multicounter_ops_for_time(T, T * mult);
        std::cout << mops << " " << std::flush;
    } std::cout << std::endl;

    for (int num_threads = T; num_threads <= T * 4; num_threads += T) {
        uint64_t mops = bench_numa_multicounter_ops_for_time(num_threads, 1);
        std::cout << mops << " " << std::flush;
    } std::cout << std::endl;

    std::cout << "1'000'000 increments by each thread" << std::endl;
    for (int num_threads = T; num_threads <= T * 4; num_threads += T) {
        bench_and_print_numa_multicounter_time_for_ops(num_threads, 1);
        for (int mult = 1; mult <= 4; mult++) {
            bench_and_print_numa_multicounter_time_for_ops(num_threads, T * mult);
        }
    }

    return 0;
}