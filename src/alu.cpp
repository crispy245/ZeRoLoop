#include "register.h"
#include "alu.h"
#include <iostream>


void ALU::half_adder(bit& s, bit& c, bit a, bit b) {
    s = a ^ b;
    c = a & b;
}

void ALU::full_adder(bit& s, bit& c, bit a, bit b, bit cin) {
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

Register ALU::add(Register& ret, Register a, Register b) {
    bit c;
    for (bigint i = 0; i < a.width(); i++) {
        full_adder(ret.at(i), c, a.at(i), b.at(i), c);
    }
    return ret;
}

Register ALU::two_complement(Register b) {
    Register complement(b.width());

    // Invert all bits (one's complement)
    for (size_t i = 0; i < b.width(); ++i) {
        complement.at(i) = ~b.at(i);
    }

    Register register_holding_1(1, b.width());
    add(complement, complement, register_holding_1);

    return complement;
}

Register ALU::subtract(Register& result, Register a, Register b) {
    Register b_complement = two_complement(b);
    bit carry = bit(0);
    add(result, a, b_complement);
    return result;
}

Register ALU::logical_shift_left(Register a, Register shift_amount) {
    Register result(a.width());

    for (bigint i = 0; i < a.width(); i++) {
        bit shifted_bit = bit(0);

        bigint src_pos = i;
        for (bigint j = 0; j < shift_amount.width(); j++) {
            if (shift_amount.at(j).value()) {
                src_pos -= (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width()) {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}

Register ALU::logical_shift_right(Register a, Register shift_amount) {
    Register result(a.width());

    for (bigint i = 0; i < a.width(); i++) {
        bit shifted_bit = bit(0);

        bigint src_pos = i;
        for (bigint j = 0; j < shift_amount.width(); j++) {
            if (shift_amount.at(j).value()) {
                src_pos += (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width()) {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}

Register ALU::arithmetic_shift_right(Register a, Register shift_amount) {
    Register result(a.width());
    bit sign_bit = a.at(a.width() - 1);

    for (bigint i = 0; i < a.width(); i++) {
        bit shifted_bit = sign_bit;

        bigint src_pos = i;
        for (bigint j = 0; j < shift_amount.width(); j++) {
            if (shift_amount.at(j).value()) {
                src_pos += (1 << j);
            }
        }

        if (src_pos >= 0 && src_pos < a.width()) {
            shifted_bit = a.at(src_pos);
        }

        result.at(i) = shifted_bit;
    }
    return result;
}

bit ALU::bit_vector_compare(const std::vector<bit>& v, const std::vector<bit>& w) {
    assert(v.size() == w.size());
    bit ret = v.at(0) ^ w.at(0);
    for (bigint i = 1; i < v.size(); i++) {
        ret |= v.at(i) ^ w.at(i);
    }
    return ret;
}

Register ALU::compare_slt(Register a, Register b) {
    Register result(a.width());
    Register sub_result(a.width());

    // Perform subtraction
    subtract(sub_result, a, b);

    // Copy sign bit to result[0], all other bits are initialized to 0
    result.at(0) = sub_result.at(a.width() - 1);
    return result;
}

Register ALU::compare_sltu(Register a, Register b) {
    Register result(a.width());
    Register sub_result(a.width());
    
    subtract(sub_result, a, b);
    
    // For unsigned comparison:
    // If the highest bit is 1 after subtraction, there was a borrow,
    // meaning a < b in unsigned comparison
    result.at(0) = sub_result.at(a.width() - 1);
    return result;
}

Register ALU::execute(Register& a, Register& b, std::vector<bit> alu_op) {
    Register result(a.width());

    // Decode operation using the alu_op bits
    bit is_add = ~alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0];
    bit is_sub = ~alu_op[3] & ~alu_op[2] & ~alu_op[1] & alu_op[0];
    bit is_sll = ~alu_op[3] & ~alu_op[2] & alu_op[1] & ~alu_op[0];
    bit is_slt = ~alu_op[3] & ~alu_op[2] & alu_op[1] & alu_op[0];
    bit is_sltu = ~alu_op[3] & alu_op[2] & ~alu_op[1] & ~alu_op[0];
    bit is_xor = ~alu_op[3] & alu_op[2] & ~alu_op[1] & alu_op[0];
    bit is_srl = ~alu_op[3] & alu_op[2] & alu_op[1] & ~alu_op[0];
    bit is_sra = ~alu_op[3] & alu_op[2] & alu_op[1] & alu_op[0];
    bit is_or = alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0];
    bit is_and = alu_op[3] & ~alu_op[2] & ~alu_op[1] & alu_op[0];

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
    sltu_result = compare_sltu(a, b);

    for (bigint i = 0; i < a.width(); i++) {
        xor_result.at(i) = a.at(i) ^ b.at(i);
        or_result.at(i) = a.at(i) | b.at(i);
        and_result.at(i) = a.at(i) & b.at(i);
    }

    for (bigint i = 0; i < result.width(); i++) {
        bit temp = result.at(i);

        temp = is_sltu.mux(temp, sltu_result.at(i));
        temp = is_slt.mux(temp, slt_result.at(i));
        temp = is_sra.mux(temp, sra_result.at(i));
        temp = is_srl.mux(temp, srl_result.at(i));
        temp = is_sll.mux(temp, sll_result.at(i));
        temp = is_sub.mux(temp, sub_result.at(i));
        temp = is_add.mux(temp, add_result.at(i));
        temp = is_xor.mux(temp, (a.at(i) ^ b.at(i)));
        temp = is_or.mux(temp, (a.at(i) | b.at(i)));
        temp = is_and.mux(temp, (a.at(i) & b.at(i)));

        result.at(i) = temp;
    }

    return result;
}
