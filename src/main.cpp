#include <iostream>
#include "../include/zero_loop.h"
#include "decoder.h"
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

void test_cpu_components() {
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

    for (const auto &[val1, val2] : test_values) {
        std::cout << "\n=== Testing with values " << val1 << " and " << val2 << " ===\n";

        // Create initial CPU and set up registers
        ZeroLoop *currentCPU = new ZeroLoop();
        Register rs1(val1, 32);
        Register rs2(val2, 32);
        currentCPU->write_register(1, rs1);
        currentCPU->write_register(2, rs2);

        // Store each operation and its parameters
        struct Operation {
            std::string name;
            bit b3, b2, b1, b0;
            size_t rd;
        };

        std::vector<Operation> operations = {
            {"ADD",  bit(0), bit(0), bit(0), bit(0), 10},
            {"SUB",  bit(1), bit(0), bit(0), bit(0), 11},
            {"SLL",  bit(0), bit(1), bit(0), bit(0), 12},
            {"SLT",  bit(1), bit(1), bit(0), bit(0), 13},
            {"SLTU", bit(0), bit(0), bit(1), bit(0), 14},
            {"XOR",  bit(1), bit(0), bit(1), bit(0), 15},
            {"SRL",  bit(0), bit(1), bit(1), bit(0), 16},
            {"SRA",  bit(1), bit(1), bit(1), bit(0), 17},
            {"OR",   bit(0), bit(0), bit(0), bit(1), 18},
            {"AND",  bit(1), bit(0), bit(0), bit(1), 19}
        };

        // Execute each operation with a new CPU instance
        for (const auto &op : operations) {
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

void print_instruction_binary(uint32_t inst) {
    std::cout << "Binary: ";
    for(int i = 31; i >= 0; i--) {
        std::cout << ((inst >> i) & 1);
        if (i % 4 == 0) std::cout << " ";
    }
    std::cout << "\n";
}

void print_alu_op(const std::vector<bit>& alu_op) {
    std::cout << "ALU op: ";
    for (const auto& b : alu_op) {
        std::cout << b.value();
    }
    std::cout << "\n";
}

void test_decoder() {
    Decoder decoder;
    ZeroLoop* CPU = new ZeroLoop();  // For ALU operations
    
    struct TestCase {
        uint32_t instruction;
        std::string name;
        size_t expected_rd;
        size_t expected_rs1;
        size_t expected_rs2;
        std::vector<bit> expected_alu_op;
        int32_t rs1_val;   // Test value for rs1
        int32_t rs2_val;   // Test value for rs2
    };

    std::vector<TestCase> tests = {
        // ADD x1, x2, x3
        {0x003100B3, "ADD", 1, 2, 3, {bit(0), bit(0), bit(0), bit(0)}, 5, 3},  // 5 + 3 = 8
        
        // SUB x2, x3, x4
        {0x40418133, "SUB", 2, 3, 4, {bit(1), bit(0), bit(0), bit(0)}, 10, 4},  // 10 - 4 = 6
        
        // XOR x3, x4, x5
        {0x005241B3, "XOR", 3, 4, 5, {bit(1), bit(0), bit(1), bit(0)}, 0xF, 0x3},  // 1111 XOR 0011 = 1100
        
        // OR x4, x5, x6
        {0x0062E233, "OR", 4, 5, 6, {bit(0), bit(0), bit(0), bit(1)}, 0x5, 0x3},  // 0101 OR 0011 = 0111
        
        // AND x5, x6, x7
        {0x007372B3, "AND", 5, 6, 7, {bit(1), bit(0), bit(0), bit(1)}, 0xF, 0x3},  // 1111 AND 0011 = 0011
        
        // SLL x6, x7, x8
        {0x00839333, "SLL", 6, 7, 8, {bit(0), bit(1), bit(0), bit(0)}, 0x1, 2},  // 1 << 2 = 4
        
        // SRL x7, x8, x9
        {0x009423B3, "SRL", 7, 8, 9, {bit(0), bit(1), bit(1), bit(0)}, 0x8, 2},  // 8 >> 2 = 2
        
        // SRA x8, x9, x10
        {0x40A4D433, "SRA", 8, 9, 10, {bit(1), bit(1), bit(1), bit(0)}, -8, 2}  // -8 >> 2 = -2
    };

    for (const auto& test : tests) {
        std::cout << "\n=== Testing " << test.name << " instruction ===\n";
        std::cout << "Instruction: 0x" << std::hex << test.instruction << std::dec << "\n";
        print_instruction_binary(test.instruction);
        
        auto decoded = decoder.decode(test.instruction);
        bool passed = true;
        
        // Test decoder results
        std::cout << "Checking decoder results:\n";
        if (decoded.rd != test.expected_rd) {
            std::cout << "X rd mismatch: got " << decoded.rd 
                     << ", expected " << test.expected_rd << "\n";
            passed = false;
        } else {
            std::cout << "✓ rd = " << decoded.rd << "\n";
        }
        
        Register rs1(test.rs1_val, 32);
        Register rs2(test.rs2_val, 32);
        Register result = CPU->execute_alu(rs1, rs2, decoded.alu_op);
        int32_t alu_result = register_to_int(result);
        
        // Calculate expected result
        int32_t expected_result;
        if (test.name == "ADD") expected_result = test.rs1_val + test.rs2_val;
        else if (test.name == "SUB") expected_result = test.rs1_val - test.rs2_val;
        else if (test.name == "XOR") expected_result = test.rs1_val ^ test.rs2_val;
        else if (test.name == "OR") expected_result = test.rs1_val | test.rs2_val;
        else if (test.name == "AND") expected_result = test.rs1_val & test.rs2_val;
        else if (test.name == "SLL") expected_result = test.rs1_val << (test.rs2_val & 0x1F);
        else if (test.name == "SRL") expected_result = (uint32_t)test.rs1_val >> (test.rs2_val & 0x1F);
        else if (test.name == "SRA") expected_result = test.rs1_val >> (test.rs2_val & 0x1F);
        
        std::cout << "\nTesting ALU operation:\n";
        std::cout << "rs1 value: " << test.rs1_val << " (0x" << std::hex << test.rs1_val << ")\n";
        std::cout << "rs2 value: " << test.rs2_val << " (0x" << std::hex << test.rs2_val << ")\n";
        std::cout << "ALU result: " << alu_result << " (0x" << std::hex << alu_result << ")\n";
        std::cout << "Expected: " << expected_result << " (0x" << std::hex << expected_result << ")\n" << std::dec;
        
        if (alu_result != expected_result) {
            std::cout << "X ALU result mismatch!\n";
            passed = false;
        } else {
            std::cout << "✓ ALU result correct\n";
        }

        if (passed) {
            std::cout << "✓ All checks passed!\n";
        } else {
            std::cout << "X Some checks failed!\n";
        }
    }
    
    delete CPU;
}

int main()
{
    test_cpu_components();
    std::cout<<"FINAL TOTAL COST:"<<total_cost<<std::endl; 
    test_decoder();
    return 0;
}