#ifndef NUMA_MQ_DUMMY_HIGH_THROUGHPUT_COUNTER_H
#define NUMA_MQ_DUMMY_HIGH_THROUGHPUT_COUNTER_H

#include <cstdint>
#include <atomic>
#include <new>
#include <numa.h>

#include "fast_random.h"
#include "abstract_counter.h"

using counter = std::atomic<uint32_t>;

struct alignas(128) aligned_counter {
    counter value;
};

class dummy_high_throughput_counter : abstract_counter {
private:
    aligned_counter * counters;
    const std::size_t num_counters;
    counter & get_counter(int thread_id) {
        return counters[thread_id].value;
    }
public:
    dummy_high_throughput_counter(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_counter;
        }
    }
    void add(int thread_id) {
        get_counter(thread_id)++;
    }
    uint32_t get(int thread_id) {
        return num_counters * get_counter(thread_id);
    }
};

#endif //NUMA_MQ_DUMMY_HIGH_THROUGHPUT_COUNTER_H
