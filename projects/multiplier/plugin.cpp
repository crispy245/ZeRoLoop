#include "plugin.h"
#include <time.h>

#define QINV -3327  // -q^(-1) mod 2^16
#define KYBER_Q 3329

void full_adder(bit &s, bit &c, bit a, bit b, bit cin)
{
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

Register barrel_shifter(Register a, Register shift_amount, bool left_or_right, bool arithmetic)
{
    bit sign = bit(arithmetic).mux(0, a.at(a.width() - 1));

    Register input(a.width());
    for (size_t i = 0; i < a.width(); i++)
    {
        input.at(i) = bit(left_or_right).mux(a.at(i), a.at(a.width() - 1 - i));
    }

    Register prev_mux_row = input;
    Register next_mux_row(input.width());
    for (size_t i = 0; i < 5; i++)
    {
        size_t shift_index = (1 << i); // 2^i: shift amount for this stage.
        for (size_t j = 0; j < a.width(); j++)
        {

            if (j >= shift_index)
            {
                next_mux_row.at(j) = shift_amount.at(i).mux(prev_mux_row.at(j), prev_mux_row.at(j - shift_index));
            }
            else
            {
                next_mux_row.at(j) = shift_amount.at(i).mux(prev_mux_row.at(j), sign);
            }
        }
        prev_mux_row = next_mux_row;
    }

    Register result(a.width());
    for (size_t i = 0; i < a.width(); i++)
    {
        result.at(i) = bit(left_or_right).mux(prev_mux_row.at(i), prev_mux_row.at(prev_mux_row.width() - 1 - i));
    }

    return result;
}

Register multiplier(Register a, Register b)
{
    // Make sure the width is not zero
    int width = a.width();
    if (width == 0) {
        return Register(width, 0);  // Return a w-bit register with value 0
    }
    
    // Initialize result register with zeros
    Register result(2 * width);
    Register shifted_a(2 * width);  // Ensure it's initialized with zeros
    
    // Copy a into the lower bits of shifted_a
    for (int i = 0; i < width; i++) {
        shifted_a.at(i) = a.at(i);
    }
    
    // For each bit in b
    for (size_t i = 0; i < width; i++) {
        // Check if the current bit of b is 1
        // Note: use value() method to get the boolean value of the bit
        if (b.at(i).value()) {
            bit carry = 0;
            // Add shifted_a to result
            for (size_t j = 0; j < 2 * width; j++) {
                bit sum, new_carry;
                full_adder(sum, new_carry, result.at(j), shifted_a.at(j), carry);
                result.at(j) = sum;
                carry = new_carry;
            }
        }
        
        // Shift shifted_a left by 1 position
        for (size_t j = 2 * width - 1; j > 0; j--) {
            shifted_a.at(j) = shifted_a.at(j - 1);
        }
        shifted_a.at(0) = 0;
    }
    
    Register low_result(32);
    for(int i = 0; i < 32; i++){
        low_result.at(i) = result.at(i);
    }
    return low_result;
}


Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    Register return_register;
    if (funct7 == 2) { // Multiplier
        return_register = multiplier(a, b);
    }
    else
        return_register = Register(0, 32);
        
    return return_register;
}