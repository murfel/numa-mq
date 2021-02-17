#include <numa.h>
#include "multicounter.h"

#ifndef NUMA_MQ_NUMA_MULTICOUNTER_H
#define NUMA_MQ_NUMA_MULTICOUNTER_H

class numa_multicounter {
private:
    static const int num_nodes = 4;
    std::array<multicounter *, num_nodes> multicounters{};

    multicounter & node_multicounter() {
        return *multicounters[numa_node_of_cpu(sched_getcpu())];
    }
public:
    explicit numa_multicounter(std::size_t num_counters_on_each_node) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(multicounter), i);
            multicounters[i] = new(data) multicounter(num_counters_on_each_node, i);
        }
    }
    void add() {
        node_multicounter().add();
    }
    uint64_t get() {
        return num_nodes * node_multicounter().get();
    }
};

#endif //NUMA_MQ_NUMA_MULTICOUNTER_H
