#ifndef NUMA_MQ_FAST_RANDOM_H
#define NUMA_MQ_FAST_RANDOM_H

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


#endif //NUMA_MQ_FAST_RANDOM_H
