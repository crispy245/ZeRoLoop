#include "full_sys.h"

bigint total_cost = 0;

int32_t register_to_int(Register &reg)
{
    int32_t result = 0;
    for (int i = 0; i < reg.width(); i++)
    {
        if (reg.at(i).value())
        {
            result |= (1 << i);
        }
    }
    return result;
}

void load_instructions(RAM *instr_mem, RAM *data_mem, const char *file_location, uint32_t data_start_addr)
{
    std::ifstream vmh_file(file_location);
    if (!vmh_file.is_open())
    {
        throw std::runtime_error("Could not open VMH file");
    }

    std::string line;
    uint32_t current_addr = 0;
    bool is_data_section = false;

    auto hex_to_uint32 = [](const std::string &hex_str) -> uint32_t
    {
        uint32_t value;
        std::stringstream ss;
        ss << std::hex << hex_str;
        ss >> value;
        return value;
    };

    auto to_bitvector = [](uint32_t value, size_t bits) -> std::vector<bit>
    {
        std::vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((value >> i) & 1));
        }
        return result;
    };

    while (std::getline(vmh_file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '@')
        {
            std::string addr_str = line.substr(1);
            current_addr = hex_to_uint32(addr_str);

            if (current_addr >= data_start_addr)
            {
                is_data_section = true;
            }
        }
        else
        {
            uint32_t value = hex_to_uint32(line);

            uint32_t word_addr = current_addr >> 2;

            std::vector<bit> value_bits = to_bitvector(value, 32);

            if (is_data_section)
            {
                std::vector<bit> addr_bits = to_bitvector((current_addr - data_start_addr) >> 2, data_mem->get_addr_bits());
                data_mem->write(addr_bits, value_bits);
                std::cout << "Loaded DATA at 0x" << std::hex << (current_addr)
                          << ": 0x" << value
                          << std::dec << std::endl;
            }
            else
            {
                std::vector<bit> addr_bits = to_bitvector(word_addr, instr_mem->get_addr_bits());
                instr_mem->write(addr_bits, value_bits);
                std::cout << "Loaded INSTRUCTION at 0x" << std::hex << current_addr
                          << ": 0x" << value
                          << std::dec << std::endl;
            }

            current_addr += 4;
        }
    }

    vmh_file.close();
}

void load_instructions(std::vector<uint32_t> &instr_mem, RAM *data_mem, const char *file_location, uint32_t data_start_addr)
{
    std::ifstream vmh_file(file_location);
    if (!vmh_file.is_open())
    {
        throw std::runtime_error("Could not open VMH file");
    }

    auto hex_to_uint32 = [](const std::string &hex_str) -> uint32_t
    {
        uint32_t value;
        std::stringstream ss;
        ss << std::hex << hex_str;
        ss >> value;
        return value;
    };

    auto to_bitvector = [](uint32_t value, size_t bits) -> std::vector<bit>
    {
        std::vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((value >> i) & 1));
        }
        return result;
    };

    std::string line;
    uint32_t current_addr = 0;
    bool is_data_section = false;

    while (std::getline(vmh_file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '@')
        {
            std::string addr_str = line.substr(1);
            current_addr = hex_to_uint32(addr_str);

            if (current_addr >= data_start_addr)
            {
                is_data_section = true;
            }
        }
        else
        {
            uint32_t value = hex_to_uint32(line);

            uint32_t word_addr = current_addr >> 2;

            std::vector<bit> value_bits = to_bitvector(value, 32);

            if (is_data_section)
            {
                std::vector<bit> addr_bits = to_bitvector((current_addr - data_start_addr) >> 2, data_mem->get_addr_bits());
                data_mem->write(addr_bits, value_bits);
                std::cout << "Loaded DATA at 0x" << std::hex << (current_addr)
                          << ": 0x" << value
                          << std::dec << std::endl;
            }
            else
            {
                instr_mem.at(word_addr) = value;
                std::cout << "Loaded INSTRUCTION at 0x" << std::hex << current_addr
                          << ": 0x" << value
                          << std::dec << std::endl;
            }

            current_addr += 4;
        }
    }

    vmh_file.close();
}

void run_full_system(char *instr_location, bool ram_accurate, bool with_decoder)
{
    bit::clear_all();
    std::cout << "\n=== Testing RISC-V CPU Implementation ===\n";

    std::vector<uint32_t> instruction_memory_fast(INSTR_MEM_SIZE);
    RAM instruction_memory_slow(INSTR_MEM_SIZE, 32);
    RAM data_memory(8192, 32);

    if (ram_accurate)
    {
        load_instructions(&instruction_memory_slow, &data_memory, instr_location, (INSTR_MEM_SIZE/4));// Since they are vmh, it will start at INSTR_MEM_SIZE/4
    }
    else
    {
        load_instructions(instruction_memory_fast, &data_memory, instr_location, (INSTR_MEM_SIZE/4));
    }

    std::cout << "\nStarting program execution:\n";
    std::cout << "===========================\n";

    ZeroLoop *currentCPU = new ZeroLoop();

    if (ram_accurate)
    {
        currentCPU->connect_memories(&instruction_memory_slow, &data_memory);
    }
    else
    {
        currentCPU->connect_memories(&instruction_memory_fast, &data_memory);
    }

    while (true)
    {
        // Check how many operations a single instruction takes
        // end_count - start_count = current instruction count
        bigint current_start_instr_gate_count = bit::ops();
        
        uint32_t current_pc = currentCPU->get_pc();

        ZeroLoop *nextCPU = new ZeroLoop();
        nextCPU->copy_state_from(*currentCPU);

        uint32_t instruction = 0;

        if (ram_accurate)
        {
            auto addr_to_bits = [](uint32_t addr, size_t bits)
            {
                std::vector<bit> result;
                for (size_t i = 0; i < bits; i++)
                {
                    result.push_back(bit((addr >> i) & 1));
                }
                return result;
            };

            std::vector<bit> pc_bits = addr_to_bits(current_pc, instruction_memory_slow.get_addr_bits());
            std::vector<bit> instr_bits = instruction_memory_slow.read(pc_bits);

            for (size_t j = 0; j < 32; j++)
            {
                if (instr_bits[j].value())
                {
                    instruction |= (1u << j);
                }
            }
        }
        else
        {
            instruction = instruction_memory_fast.at(current_pc);
        }

        if (with_decoder)
        {

            nextCPU->execute_instruction_with_decoder_optimized(instruction);
        }
        else
        {
            nextCPU->execute_instruction_without_decoder(instruction);
        }

        delete currentCPU;
        currentCPU = nextCPU;

        bigint current_end_instr_gate_count = bit::ops();
        bigint current_instr_gate_count = current_end_instr_gate_count - current_start_instr_gate_count;
    
        std::cout<< "CURRENT INSTRUCTION IS : "<<std::hex<<instruction<<std::endl;
        std::cout << "CURRENT INSTRUCTION TOOK: " << current_instr_gate_count << " GATES" << std::endl;
        nextCPU->print_registers();
        nextCPU->print_details();
    }

    delete currentCPU;
}
