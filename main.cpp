#include "cache.h"

unsigned long PC;
unsigned long length = 0;
std::vector<unsigned long> addresses;

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

    // Trace file stream
    std::ifstream input(p.trace_file);

    // Preprocess input file
    std::vector<std::string> actions;
    unsigned long address;
    std::string line, action, hex_address;

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

            // to ignore BOM characters
            if (length == 0)
            {
                std::string c_line = ss_line.str();
                action = c_line[3];
            }
            
            // hex to binary
            std::stringstream ss_hex(hex_address);
            ss_hex >> std::hex >> address;

            actions.push_back(action);
            addresses.push_back(address);

            length++;
        }
    }
    else
    {
        std::cout << p.trace_file << " not found! Please provide the correct file path." << std::endl;
        return -1;
    }

    // Create cache(s)
    Cache l1 = Cache(p.block_size, p.l1_size, p.l1_assoc, p.replacement_policy);
    Cache l2 = Cache(p.block_size, p.l2_size, p.l2_assoc, p.replacement_policy);

    bool using_l2 = l2.size > 0 && l2.assoc > 0;

    std::bitset<address_bits> bit_address;
    std::vector<unsigned long> l1_fields, l2_fields, wb_fields;
    int l1_result, l2_result;
    victim _victim;

    // Run
    for (PC=0; PC < length; PC++)
    {
        action = actions[PC];
        address = addresses[PC];

        bit_address = std::bitset<address_bits>(address);

        // decode address into tag, index, and offset fields
        l1_fields = l1.decode_address(bit_address);

        //std::cout << PC << ": " << action << ' ' <<  std::hex << address << std::dec << ":  ";
        //std::cout << std::hex << l1_fields[0] << std::dec << "  " << l1_fields[1] << "  " << l1_fields[2] << std::endl;
        
        if (action.compare(READ) == 0)
        {   
            // attempt to read from l1
            l1_result = l1.read(l1_fields);
            
            if (l1_result == READ_MISS)
            {
                // bring to l1 if read miss
                _victim = l1.allocate(l1_fields, READ);

                // write back to l2 if needed
                if (_victim.wb_needed && using_l2)
                {
                    // decode victim address and attempt to write to l2
                    l2_fields = l2.decode_address(_victim.address);
                    l2_result = l2.write(l2_fields);

                    if (l2_result == WRITE_MISS)
                    {
                        // allocate block in l2 if miss
                        _victim = l2.allocate(l2_fields, WRITE);

                        // If inclusive cache, need to invalidate the block in l1 that corresponds to l2's victim
                        if (_victim.replaced && p.inclusion_property == 1)
                        {
                            l1_fields = l1.decode_address(_victim.address);
                            l1.invalidate(l1_fields);
                        }
                    }
                }

                // issue read from l2
                if (using_l2)
                {
                    l2_fields = l2.decode_address(bit_address);
                    l2_result = l2.read(l2_fields);

                    // bring to l2 if miss
                    if (l2_result == READ_MISS)
                    {
                        _victim = l2.allocate(l2_fields, READ);

                        // invalidate victim block in l1 if necessary
                        if (_victim.replaced && p.inclusion_property == 1)
                        {
                            l1_fields = l1.decode_address(_victim.address);
                            l1.invalidate(l1_fields);
                        }
                    }
                }
            }
        }

        else if (action.compare(WRITE) == 0)
        {
            // attempt to write to l1
            l1_result = l1.write(l1_fields);

            if (l1_result == WRITE_MISS)
            {
                // bring to l1 if write misss
                _victim = l1.allocate(l1_fields, WRITE);

                // writeback to l2 if needed
                if (_victim.wb_needed && using_l2)
                {
                    l2_fields = l2.decode_address(_victim.address);
                    l2_result = l2.write(l2_fields);

                    if (l2_result == WRITE_MISS)
                    {
                        _victim = l2.allocate(l2_fields, WRITE);

                        // invalidate victim block in l1 if necessary
                        if (_victim.replaced && p.inclusion_property == 1)
                        {
                            l1_fields = l1.decode_address(_victim.address);
                            l1.invalidate(l1_fields);
                        }
                    }
                }

                // issue read from l2
                if (using_l2)
                {
                    l2_fields = l2.decode_address(bit_address);
                    l2_result = l2.read(l2_fields);

                    // bring to l2 if miss
                    if (l2_result == READ_MISS)
                    {
                        _victim = l2.allocate(l2_fields, READ);

                        // invalidate victim block in l1 if necessary
                        if (_victim.replaced && p.inclusion_property == 1)
                        {
                            l1_fields = l1.decode_address(_victim.address);
                            l1.invalidate(l1_fields);
                        }
                    }
                }   
            }
        }

        // -------------------------------------------------------------------------------------------------
        // -------------------------------------------------------------------------------------------------

        else
        {
            std::cout << " action " + action + " is not supported" << std::endl;
        }
    }

    input.close();

    // print cache contents
    std::cout << "===== L1 contents =====" << std::endl;
    print_contents(l1);

    if (using_l2)
    {   
        std::cout << "===== L2 contents =====" << std::endl;
        print_contents(l2);
    }

    // print simulation results
    print_results(l1, l2, p, using_l2);

    return 0;
}