#include <numa.h>

#include "../fast_random.h"
#include "abstract_counter.h"

#ifndef NUMA_MQ_TWO_CHOICE_AVG_H
#define NUMA_MQ_TWO_CHOICE_AVG_H

class two_choice_avg {
private:
    aligned_counter * counters;
    const std::size_t num_counters;
    counter & random_counter() {
        return counters[random_fnv1a() % num_counters].value;
    }
public:
    two_choice_avg(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_counter;
        }
    }
    void add(int thread_id) {
        counter & c1 = random_counter();
        counter & c2 = random_counter();
        uint64_t sum = c1 + c2;
        if ((sum & 1) == 0) {
            c1 = sum / 2 + 1;
            c2 = sum / 2;
        } else {
            c1 = sum / 2 + 1;
            c2 = sum / 2 + 1;
        }
    }
    uint32_t get(int thread_id) {
        return num_counters * random_counter();
    }
};

#endif //NUMA_MQ_TWO_CHOICE_AVG_H
