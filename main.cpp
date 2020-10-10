#include "cache.h"

int main(int argc, char** argv)
{
    // Capture variables 
    if (argc != 9)
    {
        std::cout << "Provide these arguments in order: " << std::endl;
        std::cout << "<BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> ";
        std::cout << "<REPLACEMENT_POLICY> <INCLUSION_PROPERTY> <trace_file>" << std::endl;
        return -1;
    }

    Params p = Params{
                    std::stoul(argv[1], NULL, 10), 
                    std::stoul(argv[2], NULL, 10),
                    std::stoul(argv[3], NULL, 10),
                    std::stoul(argv[4], NULL, 10),
                    std::stoul(argv[5], NULL, 10),
                    atoi(argv[6]),
                    atoi(argv[7]), 
                    std::string(argv[8])
    };

    print_config(p);    

    // Create cache(s)
    Cache l1 = Cache(p.block_size, p.l1_size, p.l1_assoc, p.replacement_policy, p.inclusion_property);
    Cache l2 = Cache(p.block_size, p.l2_size, p.l2_assoc, p.replacement_policy, p.inclusion_property);

    bool using_l2 = l2.size > 0 && l2.assoc > 0;

    // Trace file stream
    std::ifstream input("./traces/" + p.trace_file);

    std::string line;
    std::string action;
    std::string hex_address;
    unsigned long int_address;
    std::bitset<32> bit_address;
    int l1_result, l2_result;
    bool needs_writeback;

    int i = 0;
    if (input.is_open())
    {
        while (getline(input, line))
        {
            // ignore empty line
            if (line.empty()) 
            {
                //std::cout << "EMPTY LINE" << i << std::endl;
                continue;
            }

            // read in action and address
            std::istringstream ss_line(line);
            if (!(ss_line >> action >> hex_address)) { break; }
            
            // hex to binary
            std::stringstream ss_hex(hex_address);
            ss_hex >> std::hex >> int_address;
            bit_address = std::bitset<32>(int_address);

            // decode address into tag, index, and offset fields
            std::vector<unsigned long> l1_fields = l1.decode_address(bit_address);

            //std::cout << i << ": " << action << ' ' <<  hex_address << ":  ";
            //std::cout << std::hex << l1_fields[0] << std::dec << "  " << l1_fields[1] << "  " << l1_fields[2] << std::endl;

            // -------------------------------------------------------------------------------------------------
            // -------------------------------------------------------------------------------------------------


            if (action.compare(READ) == 0)
            {   
                l1_result = l1.read(l1_fields);
                
                if (l1_result == READ_MISS)
                {
                    // Go to next level
                    if (using_l2)
                    {

                    }
                    // With no l2
                    else
                    {
                        needs_writeback = l1.allocate(l1_fields, READ);
                    }
                }
            }

            else if (action.compare(WRITE) == 0)
            {
                l1_result = l1.write(l1_fields);

                // With no l2
                if (l1_result == WRITE_MISS)
                {
                    // Go to next level
                    if (using_l2)
                    {

                    }
                    // With no l2
                    else
                    {
                        needs_writeback = l1.allocate(l1_fields, WRITE);
                    }
                }
            }


            // -------------------------------------------------------------------------------------------------
            // -------------------------------------------------------------------------------------------------

            else
            {
                std::cout << " action " + action + " is not supported" << std::endl;
            }

            //print_contents(l1);
            //std::cout << " -------------------------------------------------------------------------------------------------" << std::endl;
            i++;
        }
    }

    input.close();

    std::cout << "===== L1 contents =====" << std::endl;
    print_contents(l1);

    if (using_l2)
    {   
        std::cout << "===== L2 contents =====" << std::endl;
        print_contents(l2);
    }

    print_results(l1, l2, using_l2);

    return 0;
}