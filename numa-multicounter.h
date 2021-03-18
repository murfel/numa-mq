#include <numa.h>
#include "counters/multicounter.h"

#ifndef NUMA_MQ_NUMA_MULTICOUNTER_H
#define NUMA_MQ_NUMA_MULTICOUNTER_H

class numa_multicounter {
private:
    const int num_nodes;
    std::vector<multicounter *> multicounters;

    multicounter & node_multicounter(int node_id) {
        return *multicounters[node_id];
    }
public:
    explicit numa_multicounter(int num_nodes, std::size_t num_counters_on_each_node) : num_nodes(num_nodes) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(multicounter), i);
            multicounters.push_back(new(data) multicounter(num_counters_on_each_node, i));
        }
    }
    void add(int node_id, int thread_id) {
        node_multicounter(node_id).add(thread_id);
    }
    uint64_t get(int node_id, int thread_id) {
        return num_nodes * node_multicounter(node_id).get(thread_id);
    }
};

#endif //NUMA_MQ_NUMA_MULTICOUNTER_H
