#include "cache.h"

void print_config(Params p)
{
    std::string str_replacement_policy, str_inclusion_property;

    switch(p.replacement_policy) {
        case 0:
            str_replacement_policy = "LRU";
            break; 
        case 1:
            str_replacement_policy = "Pseudo-LRU";
            break;
        case 2:
            str_replacement_policy = "Optimal";
            break;
    }

    switch(p.inclusion_property) {
        case 0:
            str_inclusion_property = "non-inclusive";
            break; 
        case 1:
            str_inclusion_property = "inclusive";
            break;
    }

    std::cout << "===== Simulator configuration =====" << std::endl;
    std::cout << std::left << std::setw(23) << "BLOCKSIZE:" << std::to_string(p.block_size) << std::endl;
    std::cout << std::left << std::setw(23) << "L1_SIZE: " << std::to_string(p.l1_size) << std::endl;
    std::cout << std::left << std::setw(23) << "L1_ASSOC: " << std::to_string(p.l1_assoc) << std::endl;
    std::cout << std::left << std::setw(23) << "L2_SIZE: " << std::to_string(p.l2_size) << std::endl;
    std::cout << std::left << std::setw(23) << "L2_ASSOC: " << std::to_string(p.l2_assoc) << std::endl;
    std::cout << std::left << std::setw(23) << "REPLACEMENT POLICY: " << str_replacement_policy << std::endl;
    std::cout << std::left << std::setw(23) << "INCLUSION PROPERTY: " << str_inclusion_property << std::endl;
    std::cout << std::left << std::setw(23) << "trace_file: " << p.trace_file << std::endl;
}

void print_contents(Cache c)
{
    for (int i = 0; i < c.sets; i++)
    {
        std::cout << std::left << std::setw(8) << "Set" << std::setw(4) << std::to_string(i) + ": ";
        for (int j = 0; j < c.assoc; j++)
        {
            std::cout <<  std::setw(7) << std::hex << c.cache[i][j] << std::dec << std::setw(4);

            if (c.dirty[i][j])
            {
                std::cout << "D";
            }
        }
        std::cout << std::endl;
    }
}

void print_results(Cache l1, Cache l2)
{
    std::cout << "===== Simulation results (raw) =====" << std::endl;

    std::cout << "a. number of L1 reads:        " << l1.reads << std::endl;
    std::cout << "b. number of L1 read misses:  " << l1.read_misses << std::endl;
    std::cout << "c. number of L1 writes:       " << l1.writes << std::endl;
    std::cout << "d. number of L1 write misses: " << l1.write_misses << std::endl; 
    std::cout << "e. L1 miss rate:              " << "todo" << std::endl;
    std::cout << "f. number of L1 writebacks:   " << l1.writebacks << std::endl;

    std::cout << "g. number of L2 reads:        " << l2.reads << std::endl;
    std::cout << "h. number of L2 read misses:  " << l2.read_misses << std::endl;
    std::cout << "i. number of L2 writes:       " << l2.writes << std::endl;
    std::cout << "j. number of L2 write misses: " << l2.write_misses << std::endl;
    std::cout << "k. L2 miss rate:              " << "todo" << std::endl;
    std::cout << "l. number of L2 writebacks:   " << l2.writebacks << std::endl;

    std::cout << "m. total memory traffic:      " << "todo" << std::endl;
}