#ifndef NUMA_MQ_MULTICOUNTER_H
#define NUMA_MQ_MULTICOUNTER_H


#include <cstddef>
#include <stdint-gcc.h>
#include <atomic>
#include <vector>
#include <new>
#include <numa.h>

using counter = std::atomic<uint32_t>;

inline uint64_t random_fnv1a() {
    static std::atomic<size_t> num_threads_registered{0};
    thread_local uint64_t seed = num_threads_registered++;

    const static uint64_t offset = 14695981039346656037ULL;
    const static uint64_t prime = 1099511628211;

    uint64_t hash = offset;
    hash ^= seed;
    hash *= prime;
    seed = hash;
    return hash;
}

struct alignas(128) aligned_counter {
    counter value;
};

class multicounter {
private:
    aligned_counter * counters;
    const std::size_t num_counters;
    counter & random_counter() {
        return counters[random_fnv1a() % num_counters].value;
    }
public:
    multicounter(std::size_t num_counters, int node_id) : num_counters(num_counters) {
        counters = (aligned_counter *) numa_alloc_onnode(sizeof(aligned_counter) * num_counters, node_id);
        for (std::size_t i = 0; i < num_counters; i++) {
            new(&counters[i]) aligned_counter;
        }
    }
    void add() {
        counter & c1 = random_counter();
        counter & c2 = random_counter();
        counter & c = c1 < c2 ? c1 : c2;
        c++;
    }
    uint32_t get() {
        return num_counters * random_counter();
    }
};


#endif //NUMA_MQ_MULTICOUNTER_H
