#ifndef NUMA_MQ_DUMMY_HIGH_ACCURACY_COUNTER_H
#define NUMA_MQ_DUMMY_HIGH_ACCURACY_COUNTER_H

#include <cstdint>
#include <atomic>
#include <numa.h>
#include <new>
#include "abstract_counter.h"

using counter = std::atomic<uint32_t>;

struct alignas(128) aligned_counter {
    counter value;
};

class dummy_high_accuracy_counter : abstract_counter {
private:
    aligned_counter * my_counter;
    counter & get_counter() {
        return my_counter->value;
    }
public:
    dummy_high_accuracy_counter(std::size_t num_counters, int node_id) {
        my_counter = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter), node_id);
        new(my_counter) aligned_counter;
    }
    void add(int thread_id) {
        get_counter()++;
    }
    uint32_t get(int thread_id) {
        return get_counter();
    }
};


#endif //NUMA_MQ_DUMMY_HIGH_ACCURACY_COUNTER_H
