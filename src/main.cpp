#include <iostream>
#include "../include/zero_loop.h"
#include "decoder.h"
#include "ram_cpu.h"
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

void load_instructions(RAM *instruction_memory, char *file_location)
{
    std::ifstream vmh_file(file_location);
    if (!vmh_file.is_open())
    {
        throw std::runtime_error("Could not open VMH file");
    }

    std::string line;
    uint32_t current_addr = 0;

    // Helper lambda for converting hex string to uint32_t
    auto hex_to_uint32 = [](const std::string &hex_str) -> uint32_t
    {
        // Make sure we have 8 hex digits (32 bits)
        std::string padded_hex = std::string(8 - hex_str.length(), '0') + hex_str;
        uint32_t value;
        std::stringstream ss;
        ss << std::hex << padded_hex;
        ss >> value;
        return value;
    };

    // Helper lambda for converting address/value to bit vector
    auto to_bitvector = [](uint32_t value, size_t bits) -> vector<bit>
    {
        vector<bit> result;
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
            current_addr = hex_to_uint32(addr_str) * 4;
        }
        else
        {
            uint32_t instruction = hex_to_uint32(line);

            vector<bit> addr_bits = to_bitvector(current_addr, instruction_memory->get_addr_bits());
            vector<bit> instr_bits = to_bitvector(instruction, 32);

            // Write to memory
            instruction_memory->write(addr_bits, instr_bits);

            std::cout << "Loaded instruction at 0x" << std::hex << current_addr
                      << ": 0x" << instruction
                      << std::dec << std::endl;

            current_addr += 4;
        }
        std::cout << "Raw line: " << line << std::endl;
    }
}

void test_full_system(char *instr_location)
{
    bit::clear_all();
    std::cout << "\n=== Testing RISC-V CPU Implementation ===\n";

    // Initialize memories
    RAM instruction_memory(16384, 32); //unified, should thread carefully here
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

    load_instructions(&instruction_memory, instr_location);

    std::cout << "\nStarting program execution:\n";
    std::cout << "===========================\n";

    // Initialize first CPU state
    ZeroLoop *currentCPU = new ZeroLoop();
    currentCPU->connect_memories(&instruction_memory, &instruction_memory);

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
        nextCPU->connect_memories(&instruction_memory, &instruction_memory);
        nextCPU->copy_state_from(*currentCPU);

        // Fetch instruction using current PC
        vector<bit> pc_bits = addr_to_bits(current_pc, instruction_memory.get_addr_bits());
        vector<bit> instr_bits = instruction_memory.read(pc_bits);
        std::cout << "Raw bits from memory: ";
        for (int i = 31; i >= 0; i--)
        {
            std::cout << instr_bits[i].value();
        }
        std::cout << std::endl;
        // Convert instruction bits to uint32_t
        uint32_t instruction = 0;
        for (size_t j = 0; j < 32; j++)
        {
            if (instr_bits[j].value())
            {
                instruction |= (1u << j);
            }
        }

        std::cout << "\nExecuting at PC = 0x" << current_pc
                  << ", Instruction = 0x" << std::hex << instruction << std::dec << std::endl;

        // Execute instruction on next state
        nextCPU->execute_instruction(instruction);

        // Print register state
        std::cout << "\nRegister file after instruction:\n";
        nextCPU->print_registers();

        // Print operation counts
        std::cout << "\nOperation counts:\n";
        for (auto op : bit_ops_selectors)
        {
            std::cout << bit::opsname(op) << ": " << bit::ops(op) << "\n";
        }
        std::cout << "Total operations: " << bit::ops() << "\n";
        std::cout << "===========================\n";

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