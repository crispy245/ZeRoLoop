#include "register.h"
#include "alu.h"
#include <iostream>
#include <algorithm>

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

    // Shrink the input to the size of the return register
    Register input_a(a.get_data_uint(), ret.width());
    Register input_b(b.get_data_uint(), ret.width());

    for (bigint i = 0; i < ret.width(); i++)
    {
        full_adder(ret.at(i), c, input_a.at(i), input_b.at(i), c);
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

/*

Our shifters are based off this code

module BarrelShifter #(
    parameter int DATA_WIDTH = 8,
    parameter int SHIFT_WIDTH = 3 // log2(DATA_WIDTH)
) (
    input logic [DATA_WIDTH-1:0] data_in,
    input logic [SHIFT_WIDTH-1:0] shift_amount,
    output logic [DATA_WIDTH-1:0] data_out
);

    // Internal signals for each stage of the barrel shifter
    logic [DATA_WIDTH-1:0] stage_data [0:SHIFT_WIDTH-1];

    // Assign input to first stage
    assign stage_data[0] = data_in;

    // Generate logic for each stage of the barrel shifter
    genvar i;
    for (i = 1; i < SHIFT_WIDTH; i = i + 1) begin : stage_loop
        always_comb begin
            for (int j = 0; j < DATA_WIDTH; j = j + 1) begin
                if (shift_amount[i-1]) begin
                    stage_data[i][j] = stage_data[i-1][j-1]; // Shift left
                end else begin
                    stage_data[i][j] = stage_data[i-1][j]; // No shift
                end
            end
        end
    end

    // Output is the last stage
    assign data_out = stage_data[SHIFT_WIDTH-1];

endmodule

*/

Register ALU::barrel_shifter(Register a, Register shift_amount, bool left_or_right, bool arithmetic)
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

Register ALU::logical_shift_left(Register a, Register shift_amount)
{
    return barrel_shifter(a, shift_amount, false, false);
}

Register ALU::logical_shift_right(Register a, Register shift_amount)
{
    return barrel_shifter(a, shift_amount, true, false);
}

Register ALU::arithmetic_shift_right(Register a, Register shift_amount)
{
    return barrel_shifter(a, shift_amount, true, true);
}

Register ALU::compare_slt(Register a, Register b)
{
    Register result(0, a.width());
    Register sub_result(0, a.width());
    subtract(sub_result, a, b);
    bit a_sign = a.at(a.width() - 1);
    bit b_sign = b.at(b.width() - 1);
    bit signs_differ = a_sign ^ b_sign;
    bit sub_sign = sub_result.at(a.width() - 1);

    // If signs differ, a < b iff a is negative; else, use sub_sign
    result.at(0) = signs_differ.mux(sub_sign, a_sign); // Parameters swapped here

    return result;
}

Register ALU::compare_slt(Register a, Register b, Register sub_result)
{
    Register result(0, a.width());
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
// TODO make this be supported
Register ALU::compare_sltu(Register a, Register b, Register sub_result)
{
    Register result(0, a.width());

    // Set LSB based on the MSB of subtraction result
    // If MSB is 1, it means A < B for unsigned numbers
    result.at(0) = sub_result.at(sub_result.width() - 1);
    return result;
}

Register ALU::execute(Register &a, Register &b, std::vector<bit> alu_op)
{
    Register result(a.width());

    // Decode operation using the alu_op bits
    bit is_add = ~alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0]; // 0000
    bit is_sub = alu_op[3] & ~alu_op[2] & ~alu_op[1] & ~alu_op[0];  // 0001
    bit is_sll = ~alu_op[3] & alu_op[2] & ~alu_op[1] & ~alu_op[0];  // 0010
    bit is_slt = alu_op[3] & alu_op[2] & ~alu_op[1] & ~alu_op[0];   // 1100
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
    add(add_result, a, b); // such an eyesore...
    subtract(sub_result, a, b);

    // Shifts

    /*
    We permit conditional execution of this section because we only have one single barrel shifter
    in our microarchitecture. This in turns means that every cycle we should only count it once.
    We will always count our barrel shifter (the unit responsible for shifting) once only.
    */
    if ((is_srl & 1).value())
    {
        srl_result = logical_shift_right(a, b);
    }
    else if ((is_sll & 1).value())
    {
        sll_result = logical_shift_left(a, b);
    }
    else
    {
        sra_result = arithmetic_shift_right(a, b);
    }

    // Set If Less Than's (TODO make sltu more efficient by using subs carry out)
    slt_result = compare_slt(a, b, sub_result);
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

        temp = is_and.mux(temp, and_result.at(i));
        temp = is_or.mux(temp, or_result.at(i));
        temp = is_xor.mux(temp, xor_result.at(i));
        temp = is_add.mux(temp, add_result.at(i));
        temp = is_sub.mux(temp, sub_result.at(i));
        temp = is_sll.mux(temp, sll_result.at(i));
        temp = is_srl.mux(temp, srl_result.at(i));
        temp = is_sra.mux(temp, sra_result.at(i));
        temp = is_sltu.mux(temp, sltu_result.at(i));
        temp = is_slt.mux(temp, slt_result.at(i));

        result.at(i) = temp;
    }

    return result;
}

Register ALU::execute_partial(Register &a, Register &b, std::vector<bit> alu_op)
{
    Register result(a.width());

    // Convert alu_op bits to booleans
    bool op3 = alu_op[3].value();
    bool op2 = alu_op[2].value();
    bool op1 = alu_op[1].value();
    bool op0 = alu_op[0].value();

    // Decode operations using boolean logic
    bool is_add = !op3 && !op2 && !op1 && !op0; // 0000
    bool is_sub = op3 && !op2 && !op1 && !op0;  // 1000
    bool is_sll = !op3 && op2 && !op1 && !op0;  // 0100
    bool is_slt = op3 && op2 && !op1 && !op0;   // 1100
    bool is_sltu = !op3 && !op2 && op1 && !op0; // 0010
    bool is_xor = op3 && !op2 && op1 && !op0;   // 1010
    bool is_srl = !op3 && op2 && op1 && !op0;   // 0110
    bool is_sra = op3 && op2 && op1 && !op0;    // 1110
    bool is_or = !op3 && !op2 && !op1 && op0;   // 0001
    bool is_and = op3 && !op2 && !op1 && op0;   // 1001

    if (is_add)
    {
        // std::cout << "Performing ADD operation" << std::endl;
        add(result, a, b);
    }
    else if (is_sub)
    {
        // std::cout << "Performing SUB operation" << std::endl;
        subtract(result, a, b);
    }
    else if (is_sll)
    {
        // std::cout << "Performing SLL (Shift Left Logical) operation" << std::endl;
        result = logical_shift_left(a, b);
    }
    else if (is_srl)
    {
        // std::cout << "Performing SRL (Shift Right Logical) operation" << std::endl;
        result = logical_shift_right(a, b);
    }
    else if (is_sra)
    {
        // std::cout << "Performing SRA (Shift Right Arithmetic) operation" << std::endl;
        result = arithmetic_shift_right(a, b);
    }
    else if (is_slt)
    {
        // std::cout << "Performing SLT (Set Less Than) operation" << std::endl;
        result = compare_slt(a, b);
    }
    else if (is_sltu)
    {
        // std::cout << "Performing SLTU (Set Less Than Unsigned) operation" << std::endl;
        result = compare_sltu(a, b);
    }
    else if (is_xor)
    {
        // std::cout << "Performing XOR operation" << std::endl;
        for (bigint i = 0; i < a.width(); i++)
        {
            result.at(i) = a.at(i) ^ b.at(i);
        }
    }
    else if (is_or)
    {
        // std::cout << "Performing OR operation" << std::endl;
        for (bigint i = 0; i < a.width(); i++)
        {
            result.at(i) = a.at(i) | b.at(i);
        }
    }
    else if (is_and)
    {
        // std::cout << "Performing AND operation" << std::endl;
        for (bigint i = 0; i < a.width(); i++)
        {
            result.at(i) = a.at(i) & b.at(i);
        }
    }
    else
    {
        std::cout << "No valid ALU operation selected" << std::endl;
    }

    return result;
}