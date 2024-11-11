#include <iostream>
#include "../include/zero_loop.h"


void test_cpu_components()
{
    // Clear operation counts
    bit::clear_all();

    ZeRoLoop::RegisterFile regfile(4, 4); // 32 registers of 32 bits each
    ZeRoLoop::ALU alu;
    ZeRoLoop::Instruction test_inst;

    // Test ADD operation (op = 00)
    test_inst.op = vector<bit>{bit(1), bit(0)}; // ADD operation



    // Perform ALU operation
    ZeRoLoop::Register result = alu.alu(test_inst,regfile);

    // Print results
    std::cout << "Test Results:\n";
    std::cout << "Operation: ADD\n";

    // Print operation counts
    std::cout << "\nOperation Counts:\n";
    std::cout << "Total operations: " << bit::ops() << "\n";
    std::cout << "AND operations: " << bit::ops(bit_ops_and) << "\n";
    std::cout << "OR operations: " << bit::ops(bit_ops_or) << "\n";
    std::cout << "XOR operations: " << bit::ops(bit_ops_xor) << "\n";
    std::cout << "NAND operations: " << bit::ops(bit_ops_nand) << "\n";
    std::cout << "NOT operations: " << bit::ops(bit_ops_not) << "\n";



}

int main()
{
    test_cpu_components();
}