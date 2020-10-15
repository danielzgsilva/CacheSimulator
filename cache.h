#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <bitset>
#include <vector>
#include <deque>

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

extern unsigned long PC;
extern unsigned long length;
extern std::vector<unsigned long> addresses;

struct Params
{
    unsigned long block_size;
    unsigned long l1_size, l1_assoc;
    unsigned long l2_size, l2_assoc;

    // 0 - LRU, 1 - PLRU, 2 - Optimal
    int replacement_policy;

    // 0 - non-inclusive 1 - inclusive

    int inclusion_property;
    std::string trace_file;
};

struct victim
{
    bool replaced;
    bool wb_needed;
    std::bitset<address_bits> address;
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

struct TreeNode  
{ 
  bool flag; 
  bool leaf;
  long way;

  struct TreeNode *parent; 
  struct TreeNode *left; 
  struct TreeNode *right; 

  TreeNode(bool _flag, bool _leaf, long _way, TreeNode* _parent, TreeNode* _left, TreeNode* _right) 
    { 
        flag = _flag; 
        leaf = _leaf;
        way = _way;

        parent = _parent; 
        left = _left; 
        right = _right; 
    } 
};

class LRU_Tree
{
    public:
        unsigned long sets;
        unsigned long assoc;

        // stores pointers to the root of each tree
        // we keep one tree for each set in cache
        std::vector<TreeNode*> roots;

        // stores pointers to the leaf nodes of each tree
        std::vector<std::vector<TreeNode*>> leafs;

        LRU_Tree(unsigned long sets, unsigned long assoc);
        LRU_Tree() = default;

        void update_on_access(unsigned long index, unsigned long way);
        void update_on_allocate(unsigned long index, unsigned long way);
        unsigned long get_lru_block(unsigned long index);
        void print_trees();
        void print_tree(TreeNode *root, int space);
};

class Cache
{
    public:
        unsigned long block_size, size, assoc, sets;
        int replacement_policy;
        int num_index_bits, num_offset_bits, num_tag_bits;

        LRU_Matrix lru_matrix;
        LRU_Tree lru_tree;

        // for logging
        unsigned long reads = 0; 
        unsigned long read_misses = 0; 
        unsigned long writes = 0;
        unsigned long write_misses = 0;
        unsigned long writebacks = 0;
        unsigned long direct_writebacks = 0;

        std::vector<std::vector<unsigned long>> cache;
        std::vector<std::vector<bool>> open;
        std::vector<std::vector<bool>> dirty;

        Cache(unsigned long block_size, unsigned long size, unsigned long assoc, int replacement);

        std::vector<unsigned long> decode_address(std::bitset<address_bits> address);

        std::bitset<32> encode_address(unsigned long index,unsigned long tag);

        bool tag_match(std::vector<unsigned long> address_fields, std::string action);

        long find_open_way(unsigned long index);

        unsigned long find_victim(unsigned long index);

        victim allocate(std::vector<unsigned long> address_fields, std::string action);
        
        int read(std::vector<unsigned long> address_fields);
        
        int write(std::vector<unsigned long> address_fields);
        
        void invalidate(std::vector<unsigned long> address_fields);

        unsigned long find_optimal_block(unsigned long index);
};

void print_config(Params p);
void print_contents(Cache cache);
void print_results(Cache l1, Cache l2, Params p, bool using_l2);