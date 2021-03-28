#include <new>
#include <numa.h>
#include <cstdint>
#include <atomic>

#include "../fast_random.h"
#include "abstract_counter.h"

#ifndef NUMA_MQ_TWO_CHOICE_AVG_PLUS_DELTA_H
#define NUMA_MQ_TWO_CHOICE_AVG_PLUS_DELTA_H

using counter = std::atomic<uint32_t>;

class two_choice_avg_plus_delta : abstract_counter {
private:
    aligned_counter * counters;
    const std::size_t num_counters;
    counter & random_counter() {
        return counters[random_fnv1a() % num_counters].value;
    }
public:
    two_choice_avg_plus_delta(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_counter;
        }
    }
    void add(int thread_id) {
        (void) thread_id;
        counter & c1 = random_counter();
        counter & c2 = random_counter();
        uint64_t c1_1 = c1;
        uint64_t c2_1 = c2;
        uint64_t c1_2 = c1;
        uint64_t c2_2 = c2;
        uint64_t c1_delta = c1_2 - c1_1;
        uint64_t c2_delta = c2_2 - c2_1;
        uint64_t sum = c1_2 + c2_2;
        if ((sum & 1) == 0) {
            c1 = sum / 2 + 1 + c1_delta;
            c2 = sum / 2 + c2_delta;
        } else {
            c1 = sum / 2 + 1 + c1_delta;
            c2 = sum / 2 + 1 + c2_delta;
        }
    }
    uint32_t get(int thread_id) {
        (void) thread_id;
        return num_counters * random_counter();
    }
};

#endif //NUMA_MQ_TWO_CHOICE_AVG_PLUS_DELTA_H
