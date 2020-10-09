#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <bitset>
#include <vector>

#define address_bits 32
#define READ_HIT 0
#define READ_MISS 1
#define WRITE_HIT 2
#define WRITE_MISS 3
#define ws " \t\n\r\f\v "

const std::string READ = "r";
const std::string WRITE = "w";

struct Params
{
    unsigned long block_size;
    unsigned long l1_size, l1_assoc;
    unsigned long l2_size, l2_assoc;
    int replacement_policy;
    int inclusion_property;
    std::string trace_file;
};

class Cache
{
    public:
        unsigned long block_size, size, assoc, sets;

        // 0 - LRU, 1 - PLRU, 2 - Optimal
        int replacement_policy;

        // 0 - non-inclusive 1 - inclusive
        int inclusion_property;

        int index_bits, offset_bits, tag_bits;

        // for logging
        unsigned long reads = 0; 
        unsigned long read_misses = 0; 
        unsigned long writes = 0;
        unsigned long write_misses = 0;
        unsigned long writebacks = 0;

        std::vector<std::vector<unsigned long>> cache;
        std::vector<std::vector<bool>> open;
        std::vector<std::vector<bool>> dirty;

        Cache(unsigned long block_size, unsigned long size, unsigned long assoc, int replacement, int inclusion);

        std::vector<std::string> decode_address(std::bitset<32> address);

        int read(std::vector<std::string> address_fields);
        int write(std::vector<std::string> address_fields);
        long find_open_way(unsigned long index);
};

void print_config(Params p);
void print_contents(Cache cache);
void print_results(Cache l1, Cache l2);