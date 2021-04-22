#ifndef NUMA_MQ_NUMA_MOSTLY_LOCAL_H
#define NUMA_MQ_NUMA_MOSTLY_LOCAL_H

template <class Counter>
class numa_mostly_local {
private:
    const int num_nodes;
    std::vector<Counter *> node_counters;

    const int ops_before_global = 2;
    int ops = 0;

    Counter & get_node_counter(int node_id) {
        return *node_counters[node_id];
    }

    Counter & get_random_node_counter(int node_id) {
        (void) node_id;
        static int random_node_id = 0;
        return *node_counters[random_node_id++ % num_nodes];
    }
public:
    numa_mostly_local(int num_nodes, std::size_t num_counters_on_each_node) : num_nodes(num_nodes) {
        for (int i = 0; i < num_nodes; i++) {
            auto data = numa_alloc_onnode(sizeof(Counter), i);
            node_counters.push_back(new(data) Counter(num_counters_on_each_node, i));
        }
    }
    void add(int node_id, int thread_id) {
        if (ops < ops_before_global) {
            get_node_counter(node_id).add(thread_id);
            ops++;
        } else {
            get_random_node_counter(node_id).add(thread_id);
            ops = 0;
        }
    }
    uint64_t get(int node_id, int thread_id) {
        // local get
        return num_nodes * get_node_counter(node_id).get(thread_id);
        // hi acc get
//        (void) node_id;
//        uint64_t sum = 0;
//        for (int i = 0; i < num_nodes; i++) {
//            sum += get_node_counter(i).get(thread_id);
//        }
//        return sum;
    }
};

#endif //NUMA_MQ_NUMA_MOSTLY_LOCAL_H
