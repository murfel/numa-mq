#include <numa.h>
#include "counters/two_choice_counter.h"

#ifndef NUMA_MQ_NUMA_DUMMY_HIGH_THROUGHPUT_COUNTER_H
#define NUMA_MQ_NUMA_DUMMY_HIGH_THROUGHPUT_COUNTER_H

template <class Counter>
class numa_dummy_high_throughput_counter {
private:
    const int num_nodes;
    std::vector<Counter *> node_counters;

    Counter & get_node_counter(int node_id) {
        return *node_counters[node_id];
    }
public:
    numa_dummy_high_throughput_counter(int num_nodes, std::size_t num_counters_on_each_node) : num_nodes(num_nodes) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(Counter), i);
            node_counters.push_back(new(data) two_choice_counter(num_counters_on_each_node, i));
        }
    }
    void add(int node_id, int thread_id) {
        get_node_counter(node_id).add(thread_id);
    }
    uint64_t get(int node_id, int thread_id) {
        return num_nodes * get_node_counter(node_id).get(thread_id);
    }
};

#endif //NUMA_MQ_NUMA_DUMMY_HIGH_THROUGHPUT_COUNTER_H
