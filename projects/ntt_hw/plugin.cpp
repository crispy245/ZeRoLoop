#include "plugin.h"
#include <time.h>

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

Register subtract(Register &result, Register a, Register b)
{
    Register b_complement = two_complement(b);
    bit carry = bit(0);
    add(result, a, b_complement);
    return result;
}

Register montgomery_reduce(Register a)
{

    int16_t QINV = -3327; // -q^(-1) mod 2^16
    int16_t KYBER_Q = 3329;

    int32_t a_int = (int32_t)a.get_data_uint(); // interpret as signed
    int16_t t = (int16_t)a_int * QINV;
    int32_t correction = (int32_t)t * KYBER_Q;
    int32_t a_sub_t_sw = a_int - correction;
    int16_t result_sw = a_sub_t_sw >> 16;

    Register t_times_qinv(32);
    Register QINV_R(QINV, 32);
    Register a_resized = a.get_resized(32);
    // std::cout<<"a_resized = "<<a_resized.get_data_uint()<<std::endl;
    // std::cout<<"qinv = "<<QINV_R.get_data_uint()<<std::endl;

    t_times_qinv = multiplier(a_resized, QINV_R);
    // std::cout<<"t_times_qinv HW = "<<(int16_t)t_times_qinv.get_data_uint()<<std::endl;
    // std::cout<<"t_times_qinv SW = "<<t<<std::endl;

    Register t_times_qinv_chopped((int16_t)t_times_qinv.get_data_uint(), 32);

    Register KYBER_Q_R(KYBER_Q, 32);
    Register t_times_kyberq = multiplier(t_times_qinv_chopped, KYBER_Q_R);
    // std::cout<<"t_times_kyber HW = "<<(int32_t)t_times_kyberq.get_data_uint()<<std::endl;
    // std::cout<<"t_times_kyber SW = "<<correction<<std::endl;

    Register a_sub_t(32);
    subtract(a_sub_t, a, t_times_kyberq);

    Register result(0, 32);
    for (int i = 0; i < 16; i++)
    {
        result.at(i) = a_sub_t.at(i + 16);
    }

    return result;
}

Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    Register return_register(32);
    Register multiplier_res = multiplier(a, b);
    Register mont_res = montgomery_reduce(a);

    // RNG Generator
    static unsigned int counter = 0;
    // Seed with time and a counter
    srand(time(NULL) + counter++);
    Register rand_register(rand(), 32);
    

    if (funct7 == 1)
    {   // RNG number
        return rand_register;
    }
    if (funct7 == 2)
    { // montgomery
        return_register = mont_res;
    }
    else if (funct7 == 3)
    {
        return_register = multiplier_res;
    }
    else
        return_register = Register(0, 32);

    return return_register;
}