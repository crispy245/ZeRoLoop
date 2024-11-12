#include <iostream>
#include "../include/zero_loop.h"

void print_register(const ZeRoLoop::Register &reg, const std::string &name)
{
    std::cout << name << ": ";
    for (int i = reg.data.size() - 1; i >= 0; i--)
    {
        std::cout << reg.data[i].value();
    }
    std::cout << std::endl;
}

void test_cpu_components()
{
    // Clear operation counts
    bit::clear_all();

    // Create a register file with 8-bit registers for easier viewing
    ZeRoLoop::RegisterFile regfile(4, 8);

    // Initialize test values in registers
    regfile.register_list[1].data = {bit(0), bit(1), bit(0), bit(0), bit(0), bit(0), bit(0), bit(0)};
    regfile.register_list[2].data = {bit(1), bit(0), bit(0), bit(0), bit(0), bit(0), bit(0), bit(0)};
    regfile.register_list[3].data = {bit(0), bit(0), bit(0), bit(0), bit(0), bit(0), bit(0), bit(0)};

    // Test cases
    struct TestCase
    {
        std::string name;
        vector<bit> alu_op; // 4-bit operation code
        vector<bit> rs1;    // Register select bits
        vector<bit> rs2;    // Register select bits
    };

    std::vector<TestCase> test_cases = {
        // ADD: 0000
        {"ADD", {bit(0), bit(0), bit(0), bit(0)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 + 1 = 3

        // SUB: 0001
        {"SUB", {bit(0), bit(0), bit(0), bit(1)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 - 1 = 1

        // SLL: 0010
        {"SLL", {bit(0), bit(1), bit(0), bit(0)}, {bit(1), bit(0)}, {bit(1), bit(0)}}, // 2 << 1 = 4

        // SLT: 0011
        {"SLT", {bit(1), bit(1), bit(0), bit(0)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 < 1 = 0

        // SLTU: 0100
        {"SLTU", {bit(0), bit(0), bit(1), bit(0)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 <u 1 = 0

        // XOR: 0101
        {"XOR", {bit(1), bit(0), bit(1), bit(0)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 ^ 1 = 3

        // SRL: 0110
        {"SRL", {bit(0), bit(1), bit(1), bit(0)}, {bit(1), bit(0)}, {bit(1), bit(0)}}, // 2 >> 1 = 1

        // SRA: 0111
        {"SRA", {bit(1), bit(1), bit(1), bit(0)}, {bit(1), bit(0)}, {bit(1), bit(0)}}, // 2 >>> 1 = 1

        // OR: 1000
        {"OR", {bit(0), bit(0), bit(0), bit(1)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 | 1 = 3

        // AND: 1001
        {"AND", {bit(1), bit(0), bit(0), bit(1)}, {bit(1), bit(0)}, {bit(0), bit(1)}}, // 2 & 1 = 0
    };

    bigint total_cost = 0;

    // Test each operation
    for (const auto &test : test_cases)
    {
        std::cout << "\nTesting " << test.name << " operation\n";
        std::cout << "------------------------\n";

        // Set up instruction
        ZeRoLoop::Instruction test_inst;
        test_inst.op = test.alu_op;
        test_inst.rs1 = test.rs1;
        test_inst.rs2 = test.rs2;

        // Print source register values
        print_register(regfile.register_list[1], "Register 1 (rs1)");
        print_register(regfile.register_list[2], "Register 2 (rs2)");

        // Perform operation
        ZeRoLoop::ALU alu;
        ZeRoLoop::Register rs1 = regfile.register_list[1];
        ZeRoLoop::Register rs2 = regfile.register_list[2];
        ZeRoLoop::Register result = alu.alu_execute(rs1, rs2, test.alu_op);

        // Print result
        print_register(result, "Result");

        // Print operation counts
        std::cout << "Operation counts:\n";
        std::cout << "AND: " << bit::ops(bit_ops_and)
                  << " OR: " << bit::ops(bit_ops_or)
                  << " XOR: " << bit::ops(bit_ops_xor)
                  << " NOT: " << bit::ops(bit_ops_not)
                  << " NAND: " << bit::ops(bit_ops_nand)
                  << " MUX: " << bit::ops(bit_ops_mux)
                  << " Total: " << bit::ops() << "\n";

        total_cost += bit::ops();
        bit::clear_all();
    }

    std::cout << "\nTotal gate count across all operations: " << total_cost << std::endl;
}

int main()
{
    test_cpu_components();
    return 0;
}