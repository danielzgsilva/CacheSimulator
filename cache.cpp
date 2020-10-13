#include "cache.h"

Cache::Cache(unsigned long block_size, unsigned long size, unsigned long assoc, int replacement)
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
        
        // Compute number of bits for each field
        this->num_index_bits = log2(this->sets);
        this->num_offset_bits = log2(this->block_size);
        this->num_tag_bits = address_bits - this->num_index_bits - this->num_offset_bits;

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
            //std::cout << "OPT replace" << std::endl;
        }
        else
        {
            assert(false && "the replacement policy you entered is not supported. (must be 0, 1, or 2)");
        }
    }
}

std::vector<unsigned long> Cache::decode_address(std::bitset<32> address)
{
    std::vector<unsigned long> fields(3);
    std::string string_address = address.to_string();

    // Tag field
    std::string tag = string_address.substr(0, this->num_tag_bits);

    // Index field for set lookup
    std::string index = string_address.substr(this->num_tag_bits, this->num_index_bits);

    // Block offset field
    std::string offset = string_address.substr(this->num_tag_bits+this->num_index_bits, this->num_offset_bits);

    fields[0] = std::strtoul(tag.c_str(), NULL, 2);
    fields[1] = std::strtoul(index.c_str(), NULL, 2);
    fields[2] = std::strtoul(offset.c_str(), NULL, 2);

    return fields;
}

std::bitset<32> Cache::encode_address(unsigned long index, unsigned long tag)
{
    std::bitset<address_bits> address;
    std::bitset<address_bits> tag_bits(tag);
    std::bitset<address_bits> index_bits(index);

    tag_bits <<= this->num_index_bits + this->num_offset_bits;
    index_bits <<= this->num_offset_bits;

    address = tag_bits | index_bits;

    return address;
}

bool Cache::tag_match(std::vector<unsigned long> address_fields, std::string action)
{
    unsigned long tag = address_fields[0];
    unsigned long index = address_fields[1];
    bool hit = false;

    // perform tag matching on set [index]
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->cache[index][i] == tag and this->open[index][i] == false)
        {
            hit = true;

            // update LRU matrix on hit
            if (this->replacement_policy == LRU)
            {
                // update LRU matrix after allocation
                this->lru_matrix.set_row(index, i);
                this->lru_matrix.unset_column(index, i);
            }

            // Set block to dirty if write request
            if (action.compare(WRITE) == 0)
            {
                this->dirty[index][i] = true;
            }
            //std::cout << "HIT" << i << std::endl;
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
    if (this->replacement_policy == LRU)
    {
        return this->lru_matrix.get_lru_block(index);
    }
    else if (this->replacement_policy == PLRU)
    {
        assert(false && "PLRU replacement policy not ready");
    }
    else if (this->replacement_policy == OPT)
    {
        return this->find_optimal_block(index);
    }
    else
    {
        assert(false && "unsupported replacement policy in find_victim()");
    }
}

victim Cache::allocate(std::vector<unsigned long> address_fields, std::string action)
{
    unsigned long tag = address_fields[0];
    unsigned long index = address_fields[1];

    victim v = {false, false, std::bitset<32>()};

    // Look for empty way
    long insert_way = this->find_open_way(index);

    // Found empty block so simply load into this position
    if (insert_way != -1)
    {
        this->cache[index][insert_way] = tag;
        this->open[index][insert_way] = false;
        //std::cout << "insert into empty way " << insert_way << std::endl;
    }
    else
    {   
        // find victim block
        insert_way = this->find_victim(index);

        v.replaced = true;
        v.address = this->encode_address(index, this->cache[index][insert_way]);

        // Create writeback request to next level if victim is dirty
        if (this->dirty[index][insert_way])
        {
            v.wb_needed = true;
            this->writebacks++;
        }

        // replace victim block
        this->cache[index][insert_way] = tag;
        this->dirty[index][insert_way] = false;
        this->open[index][insert_way] = false;

        //std::cout << "replace into way " << insert_way << std::endl;
    }


    if (this->replacement_policy == LRU)
    {
        // update LRU matrix after allocation
        this->lru_matrix.set_row(index, insert_way);
        this->lru_matrix.unset_column(index, insert_way);
    }

    // Mark block as dirty if write request
    if (action.compare(WRITE) == 0)
    {
        this->dirty[index][insert_way] = true;
    }

    return v;
}

void Cache::invalidate(std::vector<unsigned long> address_fields)
{
    unsigned long tag = address_fields[0];
    unsigned long index = address_fields[1];

    // perform tag matching on set [index]
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        // if block exists, invalidate it
        if (this->cache[index][i] == tag and this->open[index][i] == false)
        {
            this->open[index][i] = true;
            
            // if block is dirty, write back to main memory directly
            if (this->dirty[index][i] == true)
            {
                this->direct_writebacks++;
                this->dirty[index][i] = false;
            }
        }
    }
}

int Cache::read(std::vector<unsigned long> address_fields)
{
    this->reads++;
    bool hit = this->tag_match(address_fields, READ);

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
    bool hit = this->tag_match(address_fields, WRITE);

    if (hit)
    {
        return WRITE_HIT;
    }
    else
    {
        this->write_misses++;
        return WRITE_MISS;
    }
} 