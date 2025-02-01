#include "register.h"
#include "alu.h"
#include <iostream>

void ALU::half_adder(bit &s, bit &c, bit a, bit b)
{
    s = a ^ b;
    c = a & b;
}

void ALU::full_adder(bit &s, bit &c, bit a, bit b, bit cin)
{
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

Register ALU::add(Register &ret, Register a, Register b)
{
    bit c = bit(0); // Initialize carry to 0
    for (bigint i = 0; i < a.width(); i++)
    {
        full_adder(ret.at(i), c, a.at(i), b.at(i), c);
    }
    return ret;
}

Register ALU::two_complement(Register b)
{
    Register complement(b.width());

    // Invert all bits (one's complement)
    for (size_t i = 0; i < b.width(); ++i)
    {
        complement.at(i) = ~b.at(i);
    }

    Register register_holding_1(1, b.width());
    add(complement, complement, register_holding_1);

    return complement;
}

Register ALU::subtract(Register &result, Register a, Register b)
{
    Register b_complement = two_complement(b);
    bit carry = bit(0);
    add(result, a, b_complement);
    return result;
}

Register ALU::logical_shift_left(Register a, Register shift_amount)
{
    Register result(a.width());

    for (bigint i = 0; i < a.width(); i++)
    {
        bit shifted_bit = bit(0);
        bigint src_pos = i;

        // Only loop through the lower 5 bits of shift_amount
        for (bigint j = 0; j < 5; j++) // Changed from shift_amount.width() to 5
        {
            if (shift_amount.at(j).value())
            {
                src_pos -= (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width())
        {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}
Register ALU::logical_shift_right(Register a, Register shift_amount)
{
    Register result(a.width());

    for (bigint i = 0; i < a.width(); i++)
    {
        bit shifted_bit = bit(0);

        bigint src_pos = i;
        // Only consider lower 5 bits for 32-bit shifts
        for (bigint j = 0; j < 5; j++) // Changed from shift_amount.width() to 5
        {
            if (shift_amount.at(j).value())
            {
                src_pos += (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width())
        {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}

Register ALU::arithmetic_shift_right(Register a, Register shift_amount)
{
    Register result(a.width());
    bit sign_bit = a.at(a.width() - 1);

    for (bigint i = 0; i < a.width(); i++)
    {
        bit shifted_bit = sign_bit;

        bigint src_pos = i;
        // Only consider lower 5 bits for 32-bit shifts
        for (bigint j = 0; j < 5; j++) // Changed from shift_amount.width() to 5
        {
            if (shift_amount.at(j).value())
            {
                src_pos += (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width())
        {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}

bit ALU::bit_vector_compare(const std::vector<bit> &v, const std::vector<bit> &w)
{
    assert(v.size() == w.size());
    bit ret = v.at(0) ^ w.at(0);
    for (bigint i = 1; i < v.size(); i++)
    {
        ret |= v.at(i) ^ w.at(i);
    }
    return ret;
}

Register ALU::compare_slt(Register a, Register b)
{
    Register result(a.width());
    Register sub_result(a.width());
    subtract(sub_result, a, b);

    bit a_sign = a.at(a.width() - 1);
    bit b_sign = b.at(b.width() - 1);
    bit signs_differ = a_sign ^ b_sign;
    bit sub_sign = sub_result.at(a.width() - 1);

    // If signs differ, a < b iff a is negative; else, use sub_sign
    result.at(0) = signs_differ.mux(sub_sign, a_sign); // Parameters swapped here

    return result;
}

Register ALU::compare_sltu(Register a, Register b)
{
    Register result(a.width());
    bool a_less = false;
    bool found_diff = false;

    // Compare bits from MSB to LSB to find the first differing bit
    for (int i = a.width() - 1; i >= 0; --i)
    {
        bit a_bit = a.at(i);
        bit b_bit = b.at(i);

        if (a_bit.value() != b_bit.value())
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
Register ALU::execute(Register &a, Register &b, std::vector<bit> alu_op)
{
    Register result(a.width());

    // Decode operation using the alu_op bits
    bit is_add = ~alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0]; // 0000
    bit is_sub = alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0];  // 0001
    bit is_sll = ~alu_op[3] & alu_op[2] & ~alu_op[1] & ~alu_op[0];  // 0010
    bit is_slt = alu_op[3] & alu_op[2] & ~alu_op[1] & ~alu_op[0];   // 1100 wierd?!
    bit is_sltu = ~alu_op[3] & ~alu_op[2] & alu_op[1] & ~alu_op[0]; // 0100
    bit is_xor = alu_op[3] & ~alu_op[2] & alu_op[1] & ~alu_op[0];   // 0101
    bit is_srl = ~alu_op[3] & alu_op[2] & alu_op[1] & ~alu_op[0];   // 0110
    bit is_sra = alu_op[3] & alu_op[2] & alu_op[1] & ~alu_op[0];    // 0111
    bit is_or = ~alu_op[3] & ~alu_op[2] & ~alu_op[1] & alu_op[0];   // 1000
    bit is_and = alu_op[3] & ~alu_op[2] & ~alu_op[1] & alu_op[0];   // 1001

    // Compute all possible results
    Register add_result(a.width());
    Register sub_result(a.width());
    Register sll_result(a.width());
    Register srl_result(a.width());
    Register sra_result(a.width());
    Register slt_result(a.width());
    Register sltu_result(a.width());
    Register xor_result(a.width());
    Register or_result(a.width());
    Register and_result(a.width());

    // Perform operations
    add(add_result, a, b);
    subtract(sub_result, a, b);
    sll_result = logical_shift_left(a, b);
    srl_result = logical_shift_right(a, b);
    sra_result = arithmetic_shift_right(a, b);
    slt_result = compare_slt(a, b);
    // slt_result.print("slt result");
    sltu_result = compare_sltu(a, b);

    for (bigint i = 0; i < a.width(); i++)
    {
        xor_result.at(i) = a.at(i) ^ b.at(i);
        or_result.at(i) = a.at(i) | b.at(i);
        and_result.at(i) = a.at(i) & b.at(i);
    }

    for (bigint i = 0; i < result.width(); i++)
    {
        bit temp = bit(0);

        // Check low-priority operations first
        temp = is_and.mux(temp, and_result.at(i));
        temp = is_or.mux(temp, or_result.at(i));
        temp = is_xor.mux(temp, xor_result.at(i));
        temp = is_add.mux(temp, add_result.at(i));
        temp = is_sub.mux(temp, sub_result.at(i));
        temp = is_sll.mux(temp, sll_result.at(i));
        temp = is_srl.mux(temp, srl_result.at(i));
        temp = is_sra.mux(temp, sra_result.at(i));
        temp = is_sltu.mux(temp, sltu_result.at(i));

        // Check SLT last (highest priority)
        temp = is_slt.mux(temp, slt_result.at(i));

        result.at(i) = temp;
    }

    return result;
}
