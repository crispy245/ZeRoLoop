#include "plugin.h"

void full_adder(bit &s, bit &c, bit a, bit b, bit cin)
{
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}


Register add(Register &ret, Register a, Register b)
{
    bit c = bit(0); // Initialize carry to 0

    // Shrink the input to the size of the return register
    Register input_a(a.get_data_uint(), ret.width());
    Register input_b(b.get_data_uint(), ret.width());

    for (bigint i = 0; i < ret.width(); i++)
    {
        full_adder(ret.at(i), c, input_a.at(i), input_b.at(i), c);
    }
    return ret;
}

Register two_complement(Register b)
{
    Register complement(b.width());

    // Invert all bits (one's complement)
    for (size_t i = 0; i < b.width(); ++i)
    {
        complement.at(i) = ~b.at(i);
    }

    Register register_holding_1(1, b.width());
    add(complement, complement, register_holding_1);
    // TODO: if its already on twos complement then just return b

    return complement;
}

Register subtract(Register &result, Register a, Register b)
{
    Register b_complement = two_complement(b);
    bit carry = bit(0);
    add(result, a, b_complement);
    return result;
}




Register multiplier(Register a, Register b)
{
    // a size must be equal to b
    assert(a.width() == b.width());

    // Make sure the width is not zero
    int width = a.width();
    if (width == 0)
    {
        return Register(width, 0); // Return a w-bit register with value 0
    }

    // Get sign of both inputs
    bit a_sign = a.at(width - 1);
    bit b_sign = b.at(width - 1);

    bit resulting_sign = a_sign ^ b_sign;

    // Initialize result registers
    Register mult_result(2 * width);
    Register shifted_a(2 * width);

    // Copy a into the lower bits of shifted_a
    for (int i = 0; i < width; i++)
    {
        shifted_a.at(i) = a.at(i);
    }

    // For each bit in b
    for (size_t i = 0; i < width; i++)
    {
        // Check if the current bit of b is 1
        if (b.at(i).value())
        {
            bit carry = 0;
            // Add shifted_a to result
            for (size_t j = 0; j < 2 * width; j++)
            {
                bit sum, new_carry;
                full_adder(sum, new_carry, mult_result.at(j), shifted_a.at(j), carry);
                mult_result.at(j) = sum;
                carry = new_carry;
            }
        }

        // Shift shifted_a left by 1 position
        for (size_t j = 2 * width - 1; j > 0; j--)
        {
            shifted_a.at(j) = shifted_a.at(j - 1);
        }
        shifted_a.at(0) = 0;
    }

    Register low_result(width);
    for (int i = 0; i < width; i++)
    {
        low_result.at(i) = mult_result.at(i);
    }

    Register low_result_twos_complement = two_complement(low_result);

    Register end_result(width);
    for (int i = 0; i < width; i++)
    {
        end_result.at(i) = resulting_sign.mux(low_result.at(i), low_result_twos_complement.at(i));
    }

    return low_result;
}


Register compare_sltu(Register a, Register b)
{
    Register result(a.width());
    bool a_less = false;
    bool found_diff = false;

    // Compare bits from MSB to LSB to find the first differing bit
    for (int i = a.width() - 1; i >= 0; --i)
    {
        bit a_bit = a.at(i);
        bit b_bit = b.at(i);

        if ((~(a_bit & b_bit)).value())
        {
            // If a's bit is 0 and b's is 1, a < b (unsigned)
            a_less = (a_bit.value() == 0);
            found_diff = true;
            break;
        }
    }

    // If all bits are equal, a is not less than b (result = 0)
    result.at(0) = bit(found_diff ? a_less : 0);
    return result;
}

Register barret_reduction(Register a)
{
    const uint32_t mod = 128;       // modulus m
    const uint32_t k = 7;           // chosen k such that 2^k >= mod (128)

    Register mod_as_reg(mod, 32);

    // μ = floor(2^(2k) / m)
    uint32_t mu_val = (1 << (2 * k)) / mod;
    Register mu(mu_val, 32);



    // q = ((a >> k) * μ) >> k
    Register shifted_a(a.get_data_uint() >> k, 32);
    Register mult_res = multiplier(shifted_a, mu);

    Register q(mult_res.get_data_uint() >> k, 32);

    // r = a - q * mod
    Register q_times_mod = multiplier(q, mod_as_reg);
    Register r(0, 32);
    subtract(r, a, q_times_mod);

    // Correct the result if it's greater than or equal to mod
    Register less_than = compare_sltu(r, mod_as_reg);

    Register corrected_r(0, 32);
    subtract(corrected_r, r, mod_as_reg);

    Register result(0, 32);
    for (int i = 0; i < 32; ++i)
    {
        // Use mux to select the correct result
        result.at(i) = less_than.at(0).mux(corrected_r.at(i), r.at(i));
    }

    return result;
}


Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    Register return_register(32);
    Register multiplier_res = multiplier(a, b);
    Register adder_res;  add(adder_res,a,b);
    Register sub_res;    subtract(sub_res,a,b);
    
    
    
    
    if(funct3 == 3 && funct7 == 11){
        return_register = barret_reduction(adder_res);
    }
    else if(funct3 == 1 && funct7 == 11){
        return_register = barret_reduction(sub_res);
    }
    else if(funct3 == 2 && funct7 == 11){
        return_register = barret_reduction(multiplier_res);
    }
    else{
        return_register = barret_reduction(a);
    }

    return return_register;
}