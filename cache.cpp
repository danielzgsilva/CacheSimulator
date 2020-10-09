#include "cache.h"

Cache::Cache(unsigned long block_size, unsigned long size, unsigned long assoc, int replacement, int inclusion)
{
    assert(block_size > 0 && (block_size & (block_size - 1)) == 0 && "block size must be positive and a power of 2");
    assert(size >= 0 && "cache size must be non-negative");
    assert(assoc >= 0 && "cache associativity must be non-negative");

    this->block_size = block_size;
    this->size = size;
    this->assoc = assoc;

    if (size > 0 and assoc > 0)
    {
        this->sets = size / (assoc * block_size);

        assert(this->sets > 0 && (this->sets & (this->sets - 1)) == 0 && "number of sets must be a power of 2");

        this->replacement_policy = replacement;
        this->inclusion_property = inclusion;
        
        // Compute number of bits for each field
        this->index_bits = log2(this->sets);
        this->offset_bits = log2(this->block_size);
        this->tag_bits = address_bits - this->index_bits - this->offset_bits;

        // Create cache given number of sets and ways
        this->cache.resize(this->sets, std::vector<unsigned long>(this->assoc));

        // To track dirty and open blocks 
        this->open.resize(this->sets, std::vector<bool>(this->assoc, true));
        this->dirty.resize(this->sets, std::vector<bool>(this->assoc, false));

        if (this->replacement_policy == LRU)
        {
            lru_matrix = LRU_Matrix(this->sets, this->assoc);
        }
        else if (this->replacement_policy == PLRU)
        {
            assert(false && "PLRU replacement policy not ready");
        }
        else if (this->replacement_policy == OPT)
        {
            assert(false && "optimal replacement policy not ready");
        }
        else
        {
            assert(false && "the replacement policy you entered is not supported. (must be 0,1, or 2)");
        }
    }
}

std::vector<unsigned long> Cache::decode_address(std::bitset<32> address)
{
    std::vector<unsigned long> fields(3);
    std::string string_address = address.to_string();

    // Tag field
    std::string tag = string_address.substr(0, this->tag_bits);

    // Index field for set lookup
    std::string index = string_address.substr(this->tag_bits, this->index_bits);

    // Block offset field
    std::string offset = string_address.substr(this->tag_bits+this->index_bits, this->offset_bits);

    fields[0] = std::strtoul(tag.c_str(), NULL, 2);
    fields[1] = std::strtoul(index.c_str(), NULL, 2);
    fields[2] = std::strtoul(offset.c_str(), NULL, 2);

    return fields;
}

bool Cache::tag_match(std::vector<unsigned long> address_fields)
{
    unsigned long tag = address_fields[0];
    unsigned long index = address_fields[1];
    unsigned long offset = address_fields[2];

    bool hit = false;

    // perform tag matching on set [index]
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->cache[index][i] == tag)
        {
            hit = true;

            // update LRU matrix on hit
            this->lru_matrix.set_row(index, i);
            this->lru_matrix.unset_column(index, i);
        }
    }
    return hit;
}

long Cache::find_open_way(unsigned long index)
{   
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->open[index][i])
        {
            return i;
        }
    }

    // No open ways
    return -1;
}

unsigned long Cache::find_victim(unsigned long index)
{   
    return this->lru_matrix.get_lru_block(index);
}

bool Cache::allocate(std::vector<unsigned long> address_fields, std::string action)
{
    bool writeback_request = false;
    unsigned long tag = address_fields[0];
    unsigned long index = address_fields[1];
    unsigned long offset = address_fields[2];

    // Look for empty way
    unsigned long insert_way = this->find_open_way(index);

    // Found empty block so simply load into this position
    if (insert_way != -1)
    {
        this->cache[index][insert_way] = tag;
        this->open[index][insert_way] = false;
    }
    else
    {   
        // find victim block
        insert_way = this->find_victim(index);

        // Create writeback request to next level if victim is dirty
        if (this->dirty[index][insert_way])
        {
            writeback_request = true;
            this->writebacks++;
        }

        // replace victim block
        this->cache[index][insert_way] = tag;
        this->cache[index][insert_way] = tag;
    }

    return writeback_request;
}

int Cache::read(std::vector<unsigned long> address_fields)
{
    this->reads++;
    bool hit = this->tag_match(address_fields);

    if (hit)
    {
        return READ_HIT;
    }
    else
    {
        this->read_misses++;
        return READ_MISS;
    }
} 

int Cache::write(std::vector<unsigned long> address_fields)
{
    this->writes++;
    bool hit = this->tag_match(address_fields);

     // Mark block as dirty if write request
    if (action.compare(WRITE) == 0)
    {
        this->dirty[index][insert_way] = true;
    }

    if (hit)
    {
        return WRITE_HIT;
    }
    else
    {
        this-write_misses++;
        return WRITE_MISS;
    }
} 