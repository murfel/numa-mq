#ifndef NUMA_MQ_TWO_CHOICE_COUNTER_H
#define NUMA_MQ_TWO_CHOICE_COUNTER_H


#include <cstddef>
#include <stdint-gcc.h>
#include <atomic>
#include <vector>
#include <new>
#include <numa.h>

#include "../fast_random.h"
#include "abstract_counter.h"

using counter = std::atomic<uint32_t>;

struct alignas(128) aligned_counter {
    counter value;
};

class two_choice_counter {
private:
    aligned_counter * counters;
    const std::size_t num_counters;
    counter & random_counter() {
        return counters[random_fnv1a() % num_counters].value;
    }
public:
    two_choice_counter(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_counter;
        }
    }
    void add(int thread_id) {
        counter & c1 = random_counter();
        counter & c2 = random_counter();
        counter & c = c1 < c2 ? c1 : c2;
        c++;
    }
    uint32_t get(int thread_id) {
        return num_counters * random_counter();
    }
};


#endif //NUMA_MQ_TWO_CHOICE_COUNTER_H