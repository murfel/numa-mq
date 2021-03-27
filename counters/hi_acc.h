#ifndef NUMA_MQ_HI_ACC_H
#define NUMA_MQ_HI_ACC_H

#include <cstdint>
#include <atomic>
#include <numa.h>
#include <new>
#include "abstract_counter.h"


class hi_acc : abstract_counter {
private:
    aligned_counter * my_counter;
    counter & get_counter() {
        return my_counter->value;
    }
public:
    hi_acc(std::size_t num_counters, int node_id) {
        (void ) num_counters;
        my_counter = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter), node_id);
        new(my_counter) aligned_counter;
    }
    void add(int thread_id) {
        (void) thread_id;
        get_counter()++;
    }
    uint32_t get(int thread_id) {
        (void) thread_id;
        return get_counter();
    }
};


#endif //NUMA_MQ_HI_ACC_H
