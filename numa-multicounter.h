#include <numa.h>
#include "multicounter.h"

#ifndef NUMA_MQ_NUMA_MULTICOUNTER_H
#define NUMA_MQ_NUMA_MULTICOUNTER_H

class numa_multicounter {
private:
    static const int num_nodes = 4;
    std::array<multicounter *, num_nodes> multicounters{};

    multicounter & node_multicounter(int node_id) {
        return *multicounters[node_id];
    }
public:
    explicit numa_multicounter(std::size_t num_counters_on_each_node) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(multicounter), i);
            multicounters[i] = new(data) multicounter(num_counters_on_each_node, i);
        }
    }
    void add(int node_id) {
        node_multicounter(node_id).add();
    }
    uint64_t get(int node_id) {
        return num_nodes * node_multicounter(node_id).get();
    }
};

#endif //NUMA_MQ_NUMA_MULTICOUNTER_H
