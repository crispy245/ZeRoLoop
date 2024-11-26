#include <iostream>
#include "../include/zero_loop.h"
#include "decoder.h"
#include "ram_cpu.h"
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

void test_single_operation(ZeroLoop *CPU, const std::string &op_name,
                           size_t rs1_idx, size_t rs2_idx, size_t rd_idx,
                           bit b3, bit b2, bit b1, bit b0)
{
    // Read from register file
    Register rs1 = CPU->read_register(rs1_idx);
    Register rs2 = CPU->read_register(rs2_idx);

    int32_t rs1_val = register_to_int(rs1);
    int32_t rs2_val = register_to_int(rs2);

    std::cout << "\nTesting " << op_name << ":\n";
    std::cout << "rs1 (x" << rs1_idx << ") = " << rs1_val << " (0x" << std::hex << rs1_val << ")\n";
    std::cout << "rs2 (x" << rs2_idx << ") = " << rs2_val << " (0x" << std::hex << rs2_val << ")\n";
    std::cout << std::dec;
    total_cost += bit::ops();
    bit::clear_all();
    std::vector<bit> op = {b3, b2, b1, b0};
    Register result = CPU->execute_alu(rs1, rs2, op);

    // Write result back to register file
    CPU->write_register(rd_idx, result);

    int32_t alu_result = register_to_int(result);

    // Calculate expected result using C++ operations
    int32_t expected;
    bool success = true;

    if (op_name == "ADD")
        expected = rs1_val + rs2_val;
    else if (op_name == "SUB")
        expected = rs1_val - rs2_val;
    else if (op_name == "AND")
        expected = rs1_val & rs2_val;
    else if (op_name == "OR")
        expected = rs1_val | rs2_val;
    else if (op_name == "XOR")
        expected = rs1_val ^ rs2_val;
    else if (op_name == "SLL")
        expected = rs1_val << (rs2_val & 0x1F);
    else if (op_name == "SRL")
        expected = (uint32_t)rs1_val >> (rs2_val & 0x1F);
    else if (op_name == "SRA")
        expected = rs1_val >> (rs2_val & 0x1F);
    else if (op_name == "SLT")
        expected = (rs1_val < rs2_val) ? 1 : 0;
    else if (op_name == "SLTU")
        expected = ((uint32_t)rs1_val < (uint32_t)rs2_val) ? 1 : 0;
    else
    {
        std::cout << "Unknown operation!\n";
        success = false;
    }

    std::cout << "ALU Result (x" << rd_idx << ") = " << alu_result << " (0x" << std::hex << alu_result << ")\n";
    std::cout << "Expected          = " << expected << " (0x" << std::hex << expected << ")\n";
    std::cout << std::dec;

    if (success && alu_result == expected)
    {
        std::cout << "✓ Test passed!\n";
    }
    else
    {
        std::cout << "✗ Test failed!\n";
    }

    std::cout << "Operations used: AND=" << bit::ops(bit_ops_and)
              << " OR=" << bit::ops(bit_ops_or)
              << " XOR=" << bit::ops(bit_ops_xor)
              << " NOT=" << bit::ops(bit_ops_not)
              << " MUX=" << bit::ops(bit_ops_mux)
              << " Total=" << bit::ops() << "\n";
}

void test_cpu_components()
{
    std::vector<std::pair<int32_t, int32_t>> test_values = {
        {1, 1},          // Simple case
        {5, 3},          // Multiple bits set
        {0, 1},          // Zero and non-zero
        {-1, 1},         // Negative number
        {1024, 1},       // Power of 2
        {0x7FFFFFFF, 1}, // Max positive
        {0x80000000, 1}, // Min negative
        {0xFFFFFFFF, 1}, // All bits set
        {42, 3},         // Random numbers
        {100, 25}        // Larger numbers
    };

    for (const auto &[val1, val2] : test_values)
    {
        std::cout << "\n=== Testing with values " << val1 << " and " << val2 << " ===\n";

        // Create initial CPU and set up registers
        ZeroLoop *currentCPU = new ZeroLoop();
        Register rs1(val1, 32);
        Register rs2(val2, 32);
        currentCPU->write_register(1, rs1);
        currentCPU->write_register(2, rs2);

        // Store each operation and its parameters
        struct Operation
        {
            std::string name;
            bit b3, b2, b1, b0;
            size_t rd;
        };

        std::vector<Operation> operations = {
            {"ADD", bit(0), bit(0), bit(0), bit(0), 10},
            {"SUB", bit(1), bit(0), bit(0), bit(0), 11},
            {"SLL", bit(0), bit(1), bit(0), bit(0), 12},
            {"SLT", bit(1), bit(1), bit(0), bit(0), 13},
            {"SLTU", bit(0), bit(0), bit(1), bit(0), 14},
            {"XOR", bit(1), bit(0), bit(1), bit(0), 15},
            {"SRL", bit(0), bit(1), bit(1), bit(0), 16},
            {"SRA", bit(1), bit(1), bit(1), bit(0), 17},
            {"OR", bit(0), bit(0), bit(0), bit(1), 18},
            {"AND", bit(1), bit(0), bit(0), bit(1), 19}};

        // Execute each operation with a new CPU instance
        for (const auto &op : operations)
        {
            // Create new CPU and copy state from previous
            ZeroLoop *nextCPU = new ZeroLoop();
            nextCPU->copy_registers_from(*currentCPU);

            // Delete old CPU
            delete currentCPU;

            // Set current to new CPU
            currentCPU = nextCPU;

            // Execute operation
            test_single_operation(currentCPU, op.name, 1, 2, op.rd,
                                  op.b3, op.b2, op.b1, op.b0);

            std::cout << "\nRegister File State after " << op.name << ":\n";
            currentCPU->print_registers();
        }

        // Cleanup final CPU
        delete currentCPU;
    }
}

void print_instruction_binary(uint32_t inst)
{
    std::cout << "Binary: ";
    for (int i = 31; i >= 0; i--)
    {
        std::cout << ((inst >> i) & 1);
        if (i % 4 == 0)
            std::cout << " ";
    }
    std::cout << "\n";
}

void print_alu_op(const std::vector<bit> &alu_op)
{
    std::cout << "ALU op: ";
    for (const auto &b : alu_op)
    {
        std::cout << b.value();
    }
    std::cout << "\n";
}

void test_decoder()
{
    Decoder decoder;
    ZeroLoop *CPU = new ZeroLoop(); // For ALU operations

    struct TestCase
    {
        uint32_t instruction;
        std::string name;
        size_t expected_rd;
        size_t expected_rs1;
        size_t expected_rs2;
        std::vector<bit> expected_alu_op;
        int32_t rs1_val; // Test value for rs1
        int32_t rs2_val; // Test value for rs2
    };

    std::vector<TestCase> tests = {
        // ADD x1, x2, x3
        {0x003100B3, "ADD", 1, 2, 3, {bit(0), bit(0), bit(0), bit(0)}, 5, 3}, // 5 + 3 = 8

        // SUB x2, x3, x4
        {0x40418133, "SUB", 2, 3, 4, {bit(1), bit(0), bit(0), bit(0)}, 10, 4}, // 10 - 4 = 6

        // XOR x3, x4, x5
        {0x005241B3, "XOR", 3, 4, 5, {bit(1), bit(0), bit(1), bit(0)}, 0xF, 0x3}, // 1111 XOR 0011 = 1100

        // OR x4, x5, x6
        {0x0062E233, "OR", 4, 5, 6, {bit(0), bit(0), bit(0), bit(1)}, 0x5, 0x3}, // 0101 OR 0011 = 0111

        // AND x5, x6, x7
        {0x007372B3, "AND", 5, 6, 7, {bit(1), bit(0), bit(0), bit(1)}, 0xF, 0x3}, // 1111 AND 0011 = 0011

        // SLL x6, x7, x8
        {0x00839333, "SLL", 6, 7, 8, {bit(0), bit(1), bit(0), bit(0)}, 0x1, 2}, // 1 << 2 = 4

        // SRL x7, x8, x9
        {0x009423B3, "SRL", 7, 8, 9, {bit(0), bit(1), bit(1), bit(0)}, 0x8, 2}, // 8 >> 2 = 2

        // SRA x8, x9, x10
        {0x40A4D433, "SRA", 8, 9, 10, {bit(1), bit(1), bit(1), bit(0)}, -8, 2} // -8 >> 2 = -2
    };

    for (const auto &test : tests)
    {
        std::cout << "\n=== Testing " << test.name << " instruction ===\n";
        std::cout << "Instruction: 0x" << std::hex << test.instruction << std::dec << "\n";
        print_instruction_binary(test.instruction);

        auto decoded = decoder.decode(test.instruction);
        bool passed = true;

        // Test decoder results
        std::cout << "Checking decoder results:\n";
        if (decoded.rd != test.expected_rd)
        {
            std::cout << "X rd mismatch: got " << decoded.rd
                      << ", expected " << test.expected_rd << "\n";
            passed = false;
        }
        else
        {
            std::cout << "✓ rd = " << decoded.rd << "\n";
        }

        Register rs1(test.rs1_val, 32);
        Register rs2(test.rs2_val, 32);
        Register result = CPU->execute_alu(rs1, rs2, decoded.alu_op);
        int32_t alu_result = register_to_int(result);

        // Calculate expected result
        int32_t expected_result;
        if (test.name == "ADD")
            expected_result = test.rs1_val + test.rs2_val;
        else if (test.name == "SUB")
            expected_result = test.rs1_val - test.rs2_val;
        else if (test.name == "XOR")
            expected_result = test.rs1_val ^ test.rs2_val;
        else if (test.name == "OR")
            expected_result = test.rs1_val | test.rs2_val;
        else if (test.name == "AND")
            expected_result = test.rs1_val & test.rs2_val;
        else if (test.name == "SLL")
            expected_result = test.rs1_val << (test.rs2_val & 0x1F);
        else if (test.name == "SRL")
            expected_result = (uint32_t)test.rs1_val >> (test.rs2_val & 0x1F);
        else if (test.name == "SRA")
            expected_result = test.rs1_val >> (test.rs2_val & 0x1F);

        std::cout << "\nTesting ALU operation:\n";
        std::cout << "rs1 value: " << test.rs1_val << " (0x" << std::hex << test.rs1_val << ")\n";
        std::cout << "rs2 value: " << test.rs2_val << " (0x" << std::hex << test.rs2_val << ")\n";
        std::cout << "ALU result: " << alu_result << " (0x" << std::hex << alu_result << ")\n";
        std::cout << "Expected: " << expected_result << " (0x" << std::hex << expected_result << ")\n"
                  << std::dec;

        if (alu_result != expected_result)
        {
            std::cout << "X ALU result mismatch!\n";
            passed = false;
        }
        else
        {
            std::cout << "✓ ALU result correct\n";
        }

        if (passed)
        {
            std::cout << "✓ All checks passed!\n";
        }
        else
        {
            std::cout << "X Some checks failed!\n";
        }
    }

    delete CPU;
}

void test_memory_operations()
{
    std::cout << "\n=== Testing Memory Operations ===\n";

    RAM ram(1024, 32); // 1024 words of 32 bits each
    ZeroLoop *CPU = new ZeroLoop();

    // Test values to store in memory
    struct MemTestCase
    {
        uint32_t address;
        int32_t value;
        std::string description;
    };

    std::vector<MemTestCase> tests = {
        {0x0, 0x12345678, "Word aligned"},
        {0x4, -42, "Negative value"},
        {0xC, 0x00000000, "All zeros"},
    };

    // Convert address to bit vector
    auto addr_to_bits = [](uint32_t addr, size_t bits)
    {
        vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((addr >> i) & 1));
        }
        return result;
    };

    // Convert value to bit vector
    auto val_to_bits = [](int32_t val, size_t bits)
    {
        vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((val >> i) & 1));
        }
        return result;
    };

    // Test store operations
    for (const auto &test : tests)
    {
        std::cout << "\nTesting store at address 0x" << std::hex << test.address
                  << " with value 0x" << test.value << std::dec << "\n";
        std::cout << "Description: " << test.description << "\n";

        vector<bit> address = addr_to_bits(test.address >> 2, ram.get_addr_bits()); // Divide by 4 for word alignment
        vector<bit> data = val_to_bits(test.value, 32);

        // Store value
        ram.write(address, data);

        // Read back and verify
        vector<bit> read_data = ram.read(address);
        Register read_data_as_register = Register(read_data);
        int32_t read_value = register_to_int(read_data_as_register);

        std::cout << "Written value: 0x" << std::hex << test.value << "\n";
        std::cout << "Read value:    0x" << read_value << std::dec << "\n";

        if (read_value == test.value)
        {
            std::cout << "✓ Memory test passed!\n";
        }
        else
        {
            std::cout << "X Memory test failed!\n";
        }
    }

    // Test sequence: Store -> Load -> ALU operation
    std::cout << "\n=== Testing Store-Load-ALU sequence ===\n";

    // Store two values
    vector<bit> addr1 = addr_to_bits(0 >> 2, ram.get_addr_bits());
    vector<bit> addr2 = addr_to_bits(4 >> 2, ram.get_addr_bits());
    vector<bit> val1 = val_to_bits(10, 32);
    vector<bit> val2 = val_to_bits(20, 32);

    ram.write(addr1, val1);
    ram.write(addr2, val2);

    // Load values
    vector<bit> loaded1 = ram.read(addr1);
    vector<bit> loaded2 = ram.read(addr2);

    Register reg1(loaded1);
    Register reg2(loaded2);

    // Perform ALU operation
    std::vector<bit> add_op = {bit(0), bit(0), bit(0), bit(0)}; // ADD operation
    Register result = CPU->execute_alu(reg1, reg2, add_op);

    int32_t final_result = register_to_int(result);
    std::cout << "Loaded value 1: " << register_to_int(reg1) << "\n";
    std::cout << "Loaded value 2: " << register_to_int(reg2) << "\n";
    std::cout << "ADD result: " << final_result << "\n";

    if (final_result == 30)
    { // 10 + 20 = 30
        std::cout << "✓ Store-Load-ALU sequence passed!\n";
    }
    else
    {
        std::cout << "X Store-Load-ALU sequence failed!\n";
    }

    delete CPU;
}

void test_instruction_memory()
{
    bit::clear_all();
    std::cout << "\n=== Testing Instruction Memory ===\n";

    RAM instruction_memory(1024, 32); // Instruction memory
    RAM data_memory(1024, 32);        // Data memory
    Decoder decoder;

    // Program: Series of instructions to test
    struct Instruction
    {
        uint32_t hex;
        std::string description;
        std::string operation;
        int32_t expected_result;
    };

    std::vector<Instruction> program = {
        // Initialize values
        {0x00300093, "addi x1, x0, 3", "0 + 3", 3}, // x1 = 3
        {0x00500113, "addi x2, x0, 5", "0 + 5", 5}, // x2 = 5
        {0x002080B3, "add x1, x1, x2", "3 + 5", 8}, // x1 = x1 + x2
        {0x40208133, "sub x2, x1, x2", "8 - 5", 3}, // x2 = x1 - x2

        {0x00209393, "slli x7, x1, 2", "8 << 2", 32}, // x7 = x1 << 2
        {0x0020d413, "srli x8, x1, 2", "8 >> 2", 2},  // x8 = x1 >> 2
        {0x4020d493, "srai x9, x1, 2", "8 >> 2", 2},  // x9 = x1 >> 2 (arithmetic)
    };

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

    auto val_to_bits = [](uint32_t val, size_t bits)
    {
        vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((val >> i) & 1));
        }
        return result;
    };

    // Write program to instruction memory
    for (size_t i = 0; i < program.size(); i++)
    {
        vector<bit> addr = addr_to_bits(i * 4, instruction_memory.get_addr_bits());
        vector<bit> instr = val_to_bits(program[i].hex, 32);
        instruction_memory.write(addr, instr);

        std::cout << "Stored instruction at 0x" << std::hex << (i * 4)
                  << ": 0x" << program[i].hex << std::dec
                  << " (" << program[i].description << ")\n";
    }

    // Execute program
    std::cout << "\nExecuting program:\n";
    std::cout << "=====================================\n";
    ZeroLoop *currentCPU = new ZeroLoop();

    for (size_t pc = 0; pc < program.size() * 4; pc += 4)
    {
        std::cout << "\nExecuting instruction at PC = " << pc << std::endl;

        // Fetch
        vector<bit> pc_bits = addr_to_bits(pc, instruction_memory.get_addr_bits());
        vector<bit> instr_bits = instruction_memory.read(pc_bits);

        uint32_t instruction = 0;
        for (size_t i = 0; i < 32; i++)
        {
            if (instr_bits[i].value())
            {
                instruction |= (1u << i);
            }
        }

        std::cout << "Fetched instruction: 0x" << std::hex << instruction << std::dec << std::endl;

        // Decode
        auto decoded = decoder.decode(instruction);
        std::cout << "Decoded instruction:" << std::endl;
        std::cout << "  is_alu_op: " << decoded.is_alu_op << std::endl;
        std::cout << "  is_immediate: " << decoded.is_immediate << std::endl;
        std::cout << "  rs1: " << decoded.rs1 << std::endl;
        std::cout << "  rs2: " << decoded.rs2 << std::endl;
        std::cout << "  rd: " << decoded.rd << std::endl;

        std::cout << " alu_op :";
        for (auto op_bit : decoded.alu_op)
        {
            std::cout << op_bit.value();
        }
        std::cout << std::endl;

        if (decoded.is_immediate)
        {
            std::cout << "  imm: " << decoded.imm << std::endl;
        }

        // Create new CPU for next instruction
        ZeroLoop *nextCPU = new ZeroLoop();
        nextCPU->copy_registers_from(*currentCPU);
        delete currentCPU;
        currentCPU = nextCPU;

        // Execute with careful checking
        if (decoded.is_alu_op)
        {
            try
            {
                Register rs1 = currentCPU->read_register(decoded.rs1);
                Register rs2(32); // Initialize with width

                if (decoded.is_immediate)
                {
                    // Initialize immediate value register
                    for (int i = 0; i < 32; i++)
                    {
                        rs2.at(i) = bit((decoded.imm >> i) & 1);
                    }
                }
                else
                {
                    rs2 = currentCPU->read_register(decoded.rs2);
                }

                // Debug print before ALU
                std::cout << "Before ALU operation:" << std::endl;
                std::cout << "  rs1 value: " << register_to_int(rs1) << std::endl;
                std::cout << "  rs2/imm value: " << (decoded.is_immediate ? decoded.imm : register_to_int(rs2)) << std::endl;

                Register result = currentCPU->execute_alu(rs1, rs2, decoded.alu_op);
                currentCPU->write_register(decoded.rd, result);

                // Print result
                std::cout << "After ALU operation:" << std::endl;
                std::cout << "  Result: " << register_to_int(result) << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cout << "Exception during execution: " << e.what() << std::endl;
                throw;
            }
        }

        std::cout << "\nRegister file state after instruction:" << std::endl;
        currentCPU->print_registers();
    }

    delete currentCPU;
}

void test_full_system()
{
    bit::clear_all();
    std::cout << "\n=== Testing RISC-V CPU Implementation ===\n";

    // Initialize memories
    RAM instruction_memory(1024, 32);
    RAM data_memory(1024, 32);

    // Program: Test sequence of instructions
    struct Instruction
    {
        uint32_t hex;
        std::string description;
    };

std::vector<Instruction> program = {
    {0x00000e13, "addi x28, x0, 0"},     // x28 = 0 (first Fibonacci number)
    {0x00100e93, "addi x29, x0, 1"},     // x29 = 1 (second Fibonacci number)
    {0x00100093, "addi x1, x0, 1"},      // Constant 1
    {0x01de01b3, "add x3, x29, x29"},     // Next Term
    {0x00900213, "addi x4, x0, 9"},      // Iterations 
    
    //loop:
    {0x000e8e33, "add x28, x29, x0"},     // t1 = t2
    {0x00018eb3, "add x29, x3, x0 "},     // t2 = Next Term
    {0x01de01b3, "add x3, x28, x29"},    //  Next Term = t1 + t2
    {0x40120233, "sub x4, x4, x1"},     // Iterations - 1
    {0xfe0218e3, "bnez x4, loop"},     // 

    {0x00a00513, "addi x10, x0, 10"},     // Write 10 to R[10]

};


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

    auto val_to_bits = [](uint32_t val, size_t bits)
    {
        vector<bit> result;
        for (size_t i = 0; i < bits; i++)
        {
            result.push_back(bit((val >> i) & 1));
        }
        return result;
    };

    // Load program into instruction memory
    for (size_t i = 0; i < program.size(); i++)
    {
        vector<bit> addr = addr_to_bits(i * 4, instruction_memory.get_addr_bits());
        vector<bit> instr = val_to_bits(program[i].hex, 32);
        instruction_memory.write(addr, instr);

        std::cout << "Loaded instruction at 0x" << std::hex << (i * 4)
                  << ": 0x" << program[i].hex << std::dec
                  << " (" << program[i].description << ")\n";
    }

    std::cout << "\nStarting program execution:\n";
    std::cout << "===========================\n";

    // Initialize first CPU state
    ZeroLoop *currentCPU = new ZeroLoop();
    currentCPU->connect_memories(&instruction_memory, &data_memory);

    // Execute program cycle by cycle
    while (true)
    { // We'll break when PC reaches end of program
        uint32_t current_pc = currentCPU->get_pc();
        if (current_pc >= program.size() * 4)
            break;

        // Create next CPU state
        ZeroLoop *nextCPU = new ZeroLoop();
        nextCPU->connect_memories(&instruction_memory, &data_memory);
        nextCPU->copy_state_from(*currentCPU);

        // Fetch instruction using current PC
        vector<bit> pc_bits = addr_to_bits(current_pc, instruction_memory.get_addr_bits());
        vector<bit> instr_bits = instruction_memory.read(pc_bits);

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
                  << ", Instruction = 0x" << instruction << std::dec << std::endl;

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

int main()
{
    test_full_system();
    std::cout << "\nTotal gate count : " << bit::ops() << "\n";
    return 0;
}