#ifndef NUMA_MQ_DUMMY_HIGH_THROUGHPUT_COUNTER_H
#define NUMA_MQ_DUMMY_HIGH_THROUGHPUT_COUNTER_H

#include <cstdint>
#include <atomic>
#include <new>
#include <numa.h>

#include "../fast_random.h"
#include "abstract_counter.h"


class dummy_high_throughput_counter : abstract_counter {
private:
    using non_atomic_counter = uint32_t;

    struct alignas(128) aligned_non_atomic_counter {
        non_atomic_counter value;
    };

    aligned_non_atomic_counter * counters;
    const std::size_t num_counters;
    non_atomic_counter & get_counter(int thread_id) {
        return counters[thread_id].value;
    }
public:
    dummy_high_throughput_counter(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_non_atomic_counter *) numa_alloc_onnode(sizeof(aligned_non_atomic_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_non_atomic_counter;
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
