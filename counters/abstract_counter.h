#ifndef NUMA_MQ_ABSTRACT_COUNTER_H
#define NUMA_MQ_ABSTRACT_COUNTER_H


using counter = std::atomic<uint32_t>;

struct alignas(128) aligned_counter {
    counter value;
};

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