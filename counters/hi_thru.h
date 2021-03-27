#ifndef NUMA_MQ_HI_THRU_H
#define NUMA_MQ_HI_THRU_H

#include <cstdint>
#include <atomic>
#include <new>
#include <numa.h>

#include "../fast_random.h"
#include "abstract_counter.h"


class hi_thru : abstract_counter {
private:
    const int threads_on_node = 18;
    using non_atomic_counter = uint32_t;

    struct alignas(128) aligned_non_atomic_counter {
        non_atomic_counter value;
    };

    aligned_non_atomic_counter * counters;
    const std::size_t num_counters;
    non_atomic_counter & get_counter(int thread_id) {
        return counters[thread_id % threads_on_node].value;
    }
public:
    hi_thru(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_non_atomic_counter *) numa_alloc_onnode(sizeof(aligned_non_atomic_counter) * threads_on_node, node_id);
        for (int i = 0; i < threads_on_node; i++) {
            new(&counters[i]) aligned_non_atomic_counter;
        }
    }
    void add(int thread_id) {
        get_counter(thread_id)++;
    }
    uint32_t get(int thread_id) {
        return threads_on_node * get_counter(thread_id);
    }
};

#endif //NUMA_MQ_HI_THRU_H
