#include <vector>
#include <cstdint>
#include <numa.h>

#ifndef NUMA_MQ_NUMA_HI_THRU_ADD_HI_ACC_GET_H
#define NUMA_MQ_NUMA_HI_THRU_ADD_HI_ACC_GET_H

template <class Counter>
class numa_hi_thru_add_hi_acc_get {
private:
    const int num_nodes;
    std::vector<Counter *> node_counters;

    Counter & get_node_counter(int node_id) {
        return *node_counters[node_id];
    }
public:
    numa_hi_thru_add_hi_acc_get(int num_nodes, std::size_t num_counters_on_each_node) : num_nodes(num_nodes) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(Counter), i);
            node_counters.push_back(new(data) Counter(num_counters_on_each_node, i));
        }
    }
    void add(int node_id, int thread_id) {
        get_node_counter(node_id).add(thread_id);
    }
    uint64_t get(int node_id, int thread_id) {
        (void) node_id;
        uint64_t sum = 0;
        for (int i = 0; i < num_nodes; i++) {
            sum += get_node_counter(i).get(thread_id);
        }
        return sum;
    }
};

#endif //NUMA_MQ_NUMA_HI_THRU_ADD_HI_ACC_GET_H
