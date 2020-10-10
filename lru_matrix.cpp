#include "cache.h"

LRU_Matrix::LRU_Matrix(unsigned long sets, unsigned long assoc)
{
    this->sets = sets;
    this->assoc = assoc;

    // Create 3d LRU matrix
    this->matrix.resize(sets);
    for (int i = 0; i < sets; i++)
    {
        this->matrix[i].resize(assoc);
        for (int j = 0; j < assoc; j++)
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
    for (int i = 0; i < this->assoc; i++)
    {
        this->matrix[index][i][way] = false;
    }
}

unsigned long LRU_Matrix::get_lru_block(unsigned long index)
{
    // row of the LRU block will be all 0's
    std::vector<bool> lru_vec(this->assoc, false);

    // Find this row
    for (int i = 0; i < this->assoc; i++)
    {
        if (this->matrix[index][i] == lru_vec)
        {
            return i;
        }
    }

    std::cout << "THIS SHOULDNT HAVE HAPPENED " << std::endl;
    std::cout << "THIS SHOULDNT HAVE HAPPENED " << std::endl;
    std::cout << "THIS SHOULDNT HAVE HAPPENED " << std::endl;
    return 0;
}