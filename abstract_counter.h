#ifndef NUMA_MQ_ABSTRACT_COUNTER_H
#define NUMA_MQ_ABSTRACT_COUNTER_H

class abstract_counter {
public:
    abstract_counter() = default;
    abstract_counter(std::size_t num_counters, int node_id) {}
    void add(int thread_id) {}
    uint32_t get(int thread_id) {
        return 0;
    }
};

#endif //NUMA_MQ_ABSTRACT_COUNTER_H