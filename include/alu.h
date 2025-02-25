#pragma once

#include "bit.h"
#include <vector>
#include <string>
#include <iostream>
#include <register.h>

class ALU
{
public:
    Register execute(Register &a, Register &b, std::vector<bit> alu_op);
    Register execute_partial(Register &a, Register &b, std::vector<bit> alu_op);
    Register add(Register &ret, Register a, Register b);
    Register subtract(Register &result, Register a, Register b);

private:
    // Arithmetic operations
    Register two_complement(Register b);

    // Shift operations
    Register logical_shift_left(Register a, Register shift_amount);
    Register logical_shift_right(Register a, Register shift_amount);
    Register arithmetic_shift_right(Register a, Register shift_amount);
    Register barrel_shifter(Register a, Register shift_amount, bool left_or_right, bool arithmetic); //left = false, right = true.
    Register barrel_shifter(Register a, Register shift_amount, bool left_or_right, bool arithmetic, bool partial); //left = false, right = true. partial = true means less decoding

    // Compare operations

    //These two are for the partial ALU
    Register compare_slt(Register a, Register b);
    Register compare_sltu(Register a, Register b);

    Register compare_slt(Register a, Register b, Register sub_result);
    Register compare_sltu(Register a, Register b, Register sub_result);

    // Helper functions
    static void half_adder(bit &s, bit &c, bit a, bit b);
    static void full_adder(bit &s, bit &c, bit a, bit b, bit cin);
    static bit bit_vector_compare(const std::vector<bit> &v, const std::vector<bit> &w);
};