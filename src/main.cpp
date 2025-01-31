#include "../include/zero_loop.h"
#include <fstream>
#include <sstream>

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

void load_instructions(vector<uint32_t> &instr_mem, RAM *data_mem, const char *file_location, uint32_t data_start_addr = 2048) {

    std::ifstream vmh_file(file_location);
    if (!vmh_file.is_open()) {
        throw std::runtime_error("Could not open VMH file");
    }


    // Helper lambda for converting hex string to uint32_t
    auto hex_to_uint32 = [](const std::string &hex_str) -> uint32_t {
        uint32_t value;
        std::stringstream ss;
        ss << std::hex << hex_str;
        ss >> value;
        return value;
    };

    // Helper lambda for converting address/value to bit vector
    auto to_bitvector = [](uint32_t value, size_t bits) -> std::vector<bit> {
        std::vector<bit> result;
        for (size_t i = 0; i < bits; i++) {
            result.push_back(bit((value >> i) & 1));
        }
        return result;
    };

    std::string line;
    uint32_t current_addr = 0;
    bool is_data_section = false;

    while (std::getline(vmh_file, line)) {
        if (line.empty()) continue;

        if (line[0] == '@') {
            // Parse address
            std::string addr_str = line.substr(1);
            current_addr = hex_to_uint32(addr_str);

            // Check if we've reached the data section
            if (current_addr >= data_start_addr) {
                is_data_section = true;
            }
        } else {
            uint32_t value = hex_to_uint32(line);

            uint32_t word_addr = current_addr >> 2;

            std::vector<bit> value_bits = to_bitvector(value, 32);

            if (is_data_section) {
                // Data Instr. would start at data_start_addr, however since it is stored as an array, we would access
                // the first block of the array instead of the 2048th one, therefore we substract that value
                // Example: If data starts at 2048 and user wants memory 2052, 
                // this in reality would be the second block of our memory array
                // So second block address is 2052 - 2048.
                std::vector<bit> addr_bits = to_bitvector((current_addr - data_start_addr) >> 2, data_mem->get_addr_bits());
                data_mem->write(addr_bits, value_bits);
                std::cout << "Loaded DATA at 0x" << std::hex << (current_addr)
                          << ": 0x" << value
                          << std::dec << std::endl;
            } else {
                instr_mem.at(word_addr) = value;
                std::cout << "Loaded INSTRUCTION at 0x" << std::hex << current_addr
                          << ": 0x" << value
                          << std::dec << std::endl;
            }

            current_addr += 4; // Move to next word
        }
    }

    vmh_file.close();
}

void test_full_system(char *instr_location)
{
    bit::clear_all();
    std::cout << "\n=== Testing RISC-V CPU Implementation ===\n";

    // Initialize memories
    vector<uint32_t> instruction_memory(16384); 
    RAM data_memory(8192, 32);

    // Helper functions
    auto addr_to_bits = [](uint32_t addr, size_t bits)
    {
        vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((addr >> i) & 1));
        }
        return result;
    };

    // load_instructions(&instruction_memory, instr_location);
    load_instructions(instruction_memory, &data_memory, instr_location);


    std::cout << "\nStarting program execution:\n";
    std::cout << "===========================\n";

    // Initialize first CPU state
    ZeroLoop *currentCPU = new ZeroLoop();
    currentCPU->connect_memories(&instruction_memory, &data_memory);

    // Execute program cycle by cycle
    while (true)
    {

        if (currentCPU->get_csr_21() == 1)
        {
            std::cout << "PASSED" << std::endl;
            break;
        }

        uint32_t current_pc = currentCPU->get_pc();

        // Create next CPU state
        ZeroLoop *nextCPU = new ZeroLoop();
        nextCPU->copy_state_from(*currentCPU);

        // Fetch instruction using current PC
        uint32_t instruction = instruction_memory.at(current_pc);


        // std::cout << "\nExecuting at PC = 0x" << current_pc
        //           << ", Instruction = 0x" << std::hex << instruction << std::dec << std::endl;

        // Execute instruction on next state
        nextCPU->execute_instruction(instruction);

        // Clean up current state and move to next
        delete currentCPU;
        currentCPU = nextCPU;
    }

    // Clean up final state
    delete currentCPU;
}

int main(int argc, char *argv[])
{
    test_full_system(argv[1]);
    std::cout << "\nTotal gate count: " << bit::ops() << "\n";
    return 0;
}