#include "cache.h"

// For optimal replacement policy
unsigned long Cache::find_optimal_block(unsigned long index)
{
    // tracks when each block in set [index] will be accessed again
    std::vector<unsigned long> access_distances(this->assoc);

    std::bitset<address_bits> bit_address;
    std::vector<unsigned long> address_fields;
    unsigned long cur_index, cur_tag;

    // Find each block in the set
    for (unsigned long way = 0; way < this->assoc; way++)
    {
        // Loop through remaining instructions until we find when it is accessed again
        for (unsigned long i = PC + 1; i < length; i++)
        {
            bit_address = std::bitset<address_bits>(addresses[i]);
            address_fields = this->decode_address(bit_address);
            cur_tag = address_fields[0];
            cur_index = address_fields[1]; 

            // found next access of block
            if (cur_index == index && cur_tag == this->cache[index][way])
            {
                // track access distance
                access_distances[way] = i - PC;
                break;
            }
        }
    }

    unsigned long furthest_distance = 0, insert_way = 0;
    for (unsigned long way = 0; way < this->assoc; way++)
    {
        // return leftmost block that is never accessed again
        if (access_distances[way] == 0)
        {
            return way;
        }
        // update block that is updated furthest in future
        else if (access_distances[way] > furthest_distance)
        {
            insert_way = way;
            furthest_distance = access_distances[way];
        }
    }

    // Return block that is accessed furthest in future
    return insert_way;
}


// For LRU policy
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


// For PLRU policy
LRU_Tree::LRU_Tree(unsigned long sets, unsigned long assoc)
{
    this->sets = sets;
    this->assoc = assoc;

    this->leafs.resize(this->sets);

    unsigned long num_nodes_to_add;
    std::deque<TreeNode*> prev_level;

    // Create an LRU tree for each set
    for (unsigned long i = 0; i < sets; i++)
    {
        unsigned long j = 0;

        do 
        {   
            num_nodes_to_add = pow(2, j);

            //std::cout << "set: " << i <<  ", adding number of nodes: " << num_nodes_to_add << std::endl;
            //std::cout << prev_level.size() << " nodes in previous level" << std::endl;

            if (num_nodes_to_add == this->assoc)
            {
                // Single node tree where root is the one available way
                if (num_nodes_to_add == 1)
                {
                    //std::cout << "single node tree" << std::endl;

                    TreeNode* root = new TreeNode(false, true, 0, NULL, NULL, NULL);
                    this->roots.push_back(root);
                    this->leafs[i].push_back(root);
                }
                // add leaf nodes which represent the ways in a set
                else
                {
                    //std::cout << "add leaf nodes" << std::endl;

                    unsigned long n = prev_level.size();
                    unsigned long way = 0;
                    for (unsigned long k = 0; k < n; k++)
                    {
                        TreeNode* parent = prev_level.front();
                        prev_level.pop_front();

                        parent->left = new TreeNode(false, true, way++, parent, NULL, NULL);
                        parent->right = new TreeNode(false, true, way++, parent, NULL, NULL);
                        prev_level.push_back(parent->left);
                        prev_level.push_back(parent->right);

                        this->leafs[i].push_back(parent->left);
                        this->leafs[i].push_back(parent->right);
                    }
                }
            }
            // add nodes to track LRU block
            else
            {
                // Create root node
                if (num_nodes_to_add == 1)
                {
                    TreeNode* root = new TreeNode(false, false, -1, NULL, NULL, NULL);
                    this->roots.push_back(root);
                    prev_level.push_back(root);
                    
                    //std::cout << "non leaf root" << std::endl;
                }
                // add intermediate level in tree
                else
                {
                    
                    //std::cout << "intermediate level" << std::endl;
                    unsigned long n = prev_level.size();
                    for (unsigned long k = 0; k < n; k++)
                    {
                        TreeNode* parent = prev_level.front();
                        prev_level.pop_front();

                        parent->left = new TreeNode(false, false, -1, parent, NULL, NULL);
                        parent->right = new TreeNode(false, false, -1, parent, NULL, NULL);
                        prev_level.push_back(parent->left);
                        prev_level.push_back(parent->right);
                    }
                }
            }
            j++;
        }
        while (num_nodes_to_add != this->assoc);
        prev_level.clear();
    }
}

// access rule: 0 - left  1 - right
void LRU_Tree::update_on_access(unsigned long index, unsigned long way)
{
    TreeNode *cur = this->leafs[index][way];

    // 1-way set so nothing to update
    if (cur->parent == NULL)
    {
        return;
    }

    TreeNode *parent;

    // backtrack from leaf node up the tree, setting flags based on access rule
    do
    {
        parent = cur->parent;

        if (parent->left == cur)
        {
            parent->flag = true;
        }
        else
        {
            parent->flag = false;
        }

        cur = parent;
    }
    // stop when reach root of tree
    while (cur != this->roots[index]);
}


// flip bits along path to allocated block
void LRU_Tree::update_on_allocate(unsigned long index, unsigned long way)
{
    TreeNode *cur = this->leafs[index][way];

    // 1-way set so nothing to update
    if (cur->parent == NULL)
    {
        return;
    }

    TreeNode *parent;

    // backtrack from insert position to root, flipping bits along the path
    do
    {
        parent = cur->parent;
        parent->flag = !parent->flag;
        cur = parent;
    }
    // stop when reach root of tree
    while (cur->parent);
}

// replacement rule: 0 - right  1 - left
unsigned long LRU_Tree::get_lru_block(unsigned long index)
{
    TreeNode *root = this->roots[index];
    
    // Traverse tree following replacement rule to find LRU block
    while (root->leaf != true && root->left && root->right)
    {
        if (!root->flag)
        {
            root = root->left;
        }
        else
        {
            root = root->right;
        }
    }

    return root->way;
}


void LRU_Tree::print_trees()
{
    unsigned long num_trees = this->roots.size();

    std::cout << num_trees << " trees:" << std::endl;

    for (unsigned long i = 0; i < num_trees; i++)
    {
        this->print_tree(this->roots[i], 0);
        std::cout << "--------------------------------------------------------------------------------------------------" << std::endl;
    }
}

void LRU_Tree::print_tree(TreeNode *root, int space)  
{  
    if (root == NULL)  {return;}
  
    space += 10;  
  
    this->print_tree(root->right, space);  
  
    std::cout << std::endl;  
    for (int i = 10; i < space; i++)  {std::cout<< " "; }

    if (root->leaf)
    {
        std::cout << root->way << std::endl;  
    }
    else
    {
        std::cout << root->flag << std::endl;  
    }
    
    
    this->print_tree(root->left, space);  
}  