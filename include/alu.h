#pragma once

#include "bit.h"
#include <vector>
#include <string>
#include <iostream>
#include <register.h>

class ALU {
public:
    enum class Operation {
        ADD = 0,
        SUB = 1,
        SLL = 2,
        SLT = 3,
        SLTU = 4,
        XOR = 5,
        SRL = 6,
        SRA = 7,
        OR = 8,
        AND = 9
    };

    Register execute(Register& a, Register& b, std::vector<bit> alu_op);

private:
    // Arithmetic operations
    Register add(Register& ret, Register a, Register b);
    Register subtract(Register& result, Register a, Register b);
    Register two_complement(Register b);

    // Shift operations
    Register logical_shift_left(Register a, Register shift_amount);
    Register logical_shift_right(Register a, Register shift_amount);
    Register arithmetic_shift_right(Register a, Register shift_amount);

    // Compare operations
    Register compare_slt(Register a, Register b);
    Register compare_sltu(Register a, Register b);

    // Helper functions
    static void half_adder(bit& s, bit& c, bit a, bit b);
    static void full_adder(bit& s, bit& c, bit a, bit b, bit cin);
    static bit bit_vector_compare(const std::vector<bit>& v, const std::vector<bit>& w);
};