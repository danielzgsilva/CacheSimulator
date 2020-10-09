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

#define READ "r"
#define WRITE "w"
#define READ_HIT 0
#define READ_MISS 1
#define WRITE_HIT 2
#define WRITE_MISS 3

// replacement policies
#define LRU 0
#define PLRU 1
#define OPT 2

// inclusion properties
#define NON_INCLUSIVE 0
#define INCLUSIVE 1

struct Params
{
    unsigned long block_size;
    unsigned long l1_size, l1_assoc;
    unsigned long l2_size, l2_assoc;
    int replacement_policy;
    int inclusion_property;
    std::string trace_file;
};

class LRU_Matrix
{
    public:
        unsigned long sets;
        unsigned long assoc;

        // 2d matrix for each set
        std::vector<std::vector<std::vector<bool>>> matrix;

        LRU_Matrix(unsigned long sets, unsigned long assoc);
        LRU_Matrix() = default;

        void set_row(unsigned long index, unsigned long way);
        void unset_column(unsigned long index, unsigned long way);
        unsigned long get_lru_block(unsigned long index);
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

        LRU_Matrix lru_matrix;

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

        std::vector<unsigned long> decode_address(std::bitset<32> address);

        bool tag_match(std::vector<unsigned long> address_fields);

        long find_open_way(unsigned long index);

        unsigned long find_victim(unsigned long index);

        bool allocate(std::vector<unsigned long> address_fields, std::string action);
        
        int read(std::vector<unsigned long> address_fields);
        
        int write(std::vector<unsigned long> address_fields);
};

void print_config(Params p);
void print_contents(Cache cache);
void print_results(Cache l1, Cache l2);