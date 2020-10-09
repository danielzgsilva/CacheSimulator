#include "cache.h"

Cache::Cache(unsigned long block_size, unsigned long size, unsigned long assoc, int replacement, int inclusion)
{
    assert(block_size > 0 && (block_size & (block_size - 1)) == 0 && "block size must be positive and a power of 2");
    assert(size >= 0 && "cache size must be positive");
    assert(assoc > 0 && "cache associativity must be positive");

    this->block_size = block_size;
    this->size = size;
    this->assoc = assoc;
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
}

std::vector<std::string> Cache::decode_address(std::bitset<32> address)
{
    std::vector<std::string> fields(3);
    std::string string_address = address.to_string();

    // Tag field
    fields[0] = string_address.substr(0, this->tag_bits);

    // Index field for set lookup
    fields[1] = string_address.substr(this->tag_bits, this->index_bits);

    // Block offset field
    fields[2] = string_address.substr(this->tag_bits+this->index_bits, this->offset_bits);

    return fields;
}

int Cache::read(std::vector<std::string> address_fields)
{
    this->reads++;

    unsigned long tag = std::strtoul(address_fields[0].c_str(), NULL, 2);
    unsigned long index = std::strtoul(address_fields[1].c_str(), NULL, 2);
    unsigned long offset = std::strtoul(address_fields[2].c_str(), NULL, 2);

    bool hit = false;

    // tag matching
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->cache[index][i] == tag)
        {
            hit = true;
            std::cout <<"hit" << std::endl;
        }
    }

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

long Cache::find_open_way(unsigned long index)
{   
    // Find an open way in set [index]
    long open_block = -1;

    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->open[index][i])
        {
            open_block = i;
        }
    }

    return open_block;
}

int Cache::write(std::vector<std::string> address_fields)
{
    unsigned long tag = std::strtoul(address_fields[0].c_str(), NULL, 2);
    unsigned long index = std::strtoul(address_fields[1].c_str(), NULL, 2);
    unsigned long offset = std::strtoul(address_fields[2].c_str(), NULL, 2);

    // Look for empty way
    unsigned long empty_way = find_open_way(index);

    // Found empty block so simply load into this position
    if (empty_way != -1)
    {
        this->cache[index][empty_way] = tag;
    }
    // Need to find victim block
    else
    {

    }

}