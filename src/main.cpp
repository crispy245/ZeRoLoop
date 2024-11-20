#include <iostream>
#include <iomanip>
#include "../include/zero_loop.h"

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

        ZeroLoop *CPU = new ZeroLoop();

        // Initialize registers x1 and x2 with test values
        Register rs1(val1, 32);
        Register rs2(val2, 32);
        CPU->write_register(1, rs1);
        CPU->write_register(2, rs2);

        // Test all ALU operations using registers
        // Format: rs1 = x1, rs2 = x2, rd starts at x10
        test_single_operation(CPU, "ADD", 1, 2, 10, bit(0), bit(0), bit(0), bit(0));
        test_single_operation(CPU, "SUB", 1, 2, 11, bit(1), bit(0), bit(0), bit(0));
        test_single_operation(CPU, "SLL", 1, 2, 12, bit(0), bit(1), bit(0), bit(0));
        test_single_operation(CPU, "SLT", 1, 2, 13, bit(0), bit(1), bit(1), bit(0));
        test_single_operation(CPU, "SLTU", 1, 2, 14, bit(1), bit(1), bit(0), bit(0));
        test_single_operation(CPU, "XOR", 1, 2, 15, bit(1), bit(0), bit(1), bit(0));
        test_single_operation(CPU, "SRL", 1, 2, 16, bit(1), bit(1), bit(1), bit(0));
        test_single_operation(CPU, "SRA", 1, 2, 17, bit(1), bit(1), bit(1), bit(1));
        test_single_operation(CPU, "OR", 1, 2, 18, bit(0), bit(0), bit(0), bit(1));
        test_single_operation(CPU, "AND", 1, 2, 19, bit(0), bit(0), bit(1), bit(1));

        std::cout << "\nFinal Register File State:\n";
        CPU->print_registers();

        // Test register file preservation
        ZeroLoop *nextCPU = new ZeroLoop();
        nextCPU->copy_registers_from(*CPU);

        std::cout << "\nRegister File State after copy:\n";
        nextCPU->print_registers();

        delete CPU;
        delete nextCPU;
    }
}

int main()
{
    test_cpu_components();
    return 0;
}