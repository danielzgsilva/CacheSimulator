#include "cache.h"

LRU_Matrix::LRU_Matrix(unsigned long sets, unsigned long assoc)
{
    this->sets = sets;
    this->assoc = assoc;

    // Create 3d LRU matrix
    this->matrix.resize(sets);
    for (unsigned long i = 0; i < sets; i++)
    {
        this->matrix[i].resize(assoc);
        for (unsigned long j = 0; j < assoc; j++)
        {
            this->matrix[i][j].resize(assoc, false);
        }
    }
}

void LRU_Matrix::set_row(unsigned long index, unsigned long way)
{
    this->matrix[index][way] = std::vector<bool>(this->assoc, true);
}

void LRU_Matrix::unset_column(unsigned long index, unsigned long way)
{
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        this->matrix[index][i][way] = false;
    }
}

unsigned long LRU_Matrix::get_lru_block(unsigned long index)
{
    // row of the LRU block will be all 0's
    std::vector<bool> lru_vec(this->assoc, false);

    // Find this row
    for (unsigned long i = 0; i < this->assoc; i++)
    {
        if (this->matrix[index][i] == lru_vec)
        {
            return i;
        }
    }

    return 0;
}

unsigned long Cache::find_optimal_block(unsigned long index)
{
    // tracks when each block in set [index] will be accessed again
    std::vector<unsigned long> access_distances(this->assoc);
    std::bitset<address_bits> bit_address;
    unsigned long cur_tag;

    unsigned long furthest_distance = 0, insert_way = 0;

    // Find each way in the set
    for (unsigned long way = 0; way < this->assoc; way++)
    {
        // Loop through remaining instructions until we find when it is accessed again
        for (unsigned long i = PC + 1; i < length; i++)
        {
            bit_address = std::bitset<address_bits>(addresses[i]);
            cur_tag = this->decode_address(bit_address)[0];

            // found next access of tag in way i
            if (cur_tag == this->cache[index][way])
            {
                access_distances[way] = i - PC;
                break;
            }
        }
    }

    // Return block that is accessed furthest in future
    for (unsigned long way = 0; way < this->assoc; way++)
    {
        // never accessed again
        if (access_distances[way] == 0)
        {
            return way;
        }
        else if (access_distances[way] > furthest_distance)
        {
            insert_way = way;
            furthest_distance = access_distances[way];
        }
    }

    return insert_way;
}