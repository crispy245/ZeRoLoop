#include "bit_cost.h"
#include "bit_vector_cost.h"
#include "bit_vector.h"
#include "bit.h"
#include "ram.h"
#include <iostream>

struct ZeRoLoop
{
        struct Register
        {
                vector<bit> data;

                Register(size_t width = 32) : data(width, bit(0)) {}
                Register(vector<bit> &bits) : data(bits) {}
                Register(bigint value, size_t width) : data(width, bit(0))
                {
                        vector<bit> value_bits = bit_vector_from_integer(value);

                        for (bigint i = 0; i < width && i < value_bits.size(); i++)
                        {
                                data[i] = value_bits[i];
                        }
                }

                bit get_bit(size_t index) const
                {
                        return data[index];
                }

                // Get number of bits in register
                size_t width() const
                {
                        return data.size();
                }

                // Print register contents
                void print(const std::string &name) const
                {
                        std::cout << name << ": ";
                        for (int i = width() - 1; i >= 0; i--)
                        {
                                std::cout << data[i].value();
                        }
                        std::cout << std::endl;
                }

                // Access individual bits with bounds checking
                bit &at(size_t index)
                {
                        return data.at(index);
                }

                const bit &at(size_t index) const
                {
                        return data.at(index);
                }

                void push_back(const bit &b)
                {
                        data.push_back(b);
                }
        };

        struct RegisterFile
        {
                vector<Register> register_list;

                RegisterFile(size_t num_registers = 32, size_t width = 32)
                    : register_list(num_registers, Register(width)) {}

                // Access registers with bounds checking
                Register &at(size_t index)
                {
                        return register_list.at(index);
                }

                const Register &at(size_t index) const
                {
                        return register_list.at(index);
                }

                size_t count() const
                {
                        return register_list.size();
                }
        };

        static Register register_read(RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits)
        {
                if (H <= L)
                        return Register{};
                if (H == L + 1)
                        return x.at(L);

                bigint splitpos = 0;
                bigint split = 1;
                while (L + split < H - split)
                {
                        splitpos += 1;
                        split *= 2;
                }

                if (ibits <= splitpos)
                {
                        return register_read(x, L, L + split, i, ibits);
                }

                Register result0 = register_read(x, L, L + split, i, splitpos);
                Register result1 = register_read(x, L + split, H, i, splitpos);

                assert(result0.width() == result1.width());

                bit isplit = i.at(splitpos);

                Register result(0);
                for (bigint r = 0; r < result0.width(); ++r)
                {
                        bit x0 = result0.at(r);
                        bit x1 = result1.at(r);
                        result.push_back(isplit.mux(x0, x1));
                }

                return result;
        }

        static Register register_read(RegisterFile &x, bigint L, bigint H, Register &i)
        {
                return register_read(x, L, H, i, i.width());
        }

        static Register register_read(RegisterFile &x, Register &i)
        {
                return register_read(x, 0, x.count(), i);
        }

        static void register_write(RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits,
                                   Register &write_data, bit b = bit(1), bool top = 1)
        {
                assert(x.at(0).width() == write_data.width());
                if (H <= L)
                        return;
                if (H == L + 1)
                {
                        if (top)
                                x.at(L) = write_data;
                        else
                                for (bigint r = 0; r < write_data.width(); ++r)
                                        x.at(L).at(r) = b.mux(write_data.at(r), x.at(L).at(r));
                        return;
                }

                bigint splitpos = 0;
                bigint split = 1;
                while (L + split < H - split)
                {
                        splitpos += 1;
                        split *= 2;
                }

                if (ibits <= splitpos)
                {
                        return register_write(x, L, L + split, i, ibits, write_data, b, top);
                }

                bit isplit = i.at(splitpos);
                register_write(x, L, L + split, i, splitpos, write_data, top ? isplit : (b | isplit), 0);
                register_write(x, L + split, H, i, splitpos, write_data, top ? ~isplit : b.orn(isplit), 0);
        }

        static void register_write(RegisterFile &x, bigint L, bigint H, Register &i, Register &data)
        {
                register_write(x, L, H, i, i.width(), data);
        }

        static void register_write(RegisterFile &x, Register &i, Register &data)
        {
                register_write(x, 0, x.count(), i, data);
        }

        struct Instruction
        {
                vector<bit> op;
                vector<bit> rs1;
                vector<bit> rs2;
                vector<bit> rd;
        };

        struct ALU
        {
                // ALU operation codes (matches RISC-V)
                enum class AluOp
                {
                        ADD = 0,  // 0000
                        SUB = 1,  // 0001
                        SLL = 2,  // 0010
                        SLT = 3,  // 0011
                        SLTU = 4, // 0100
                        XOR = 5,  // 0101
                        SRL = 6,  // 0110
                        SRA = 7,  // 0111
                        OR = 8,   // 1000
                        AND = 9,  // 1001
                };

                // Helper functions for arithmetic
                static inline void half_adder(bit &s, bit &c, bit a, bit b)
                {
                        s = a ^ b;
                        c = a & b;
                }

                static inline void full_adder(bit &s, bit &c, bit a, bit b, bit cin)
                {
                        bit t = (a ^ b);
                        s = t ^ cin;
                        c = (a & b) | (cin & t);
                }

                Register add(Register &ret, Register a, Register b)
                {
                        bit c;
                        for (bigint i = 0; i < a.width(); i++)
                        {
                                full_adder(ret.at(i), c, a.at(i), b.at(i), c);
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

                        return complement;
                }

                Register subtract(Register &result, Register a, Register b)
                {
                        Register b_complement = two_complement(b);
                        bit carry = bit(0);

                        add(result, a, b_complement);

                        return result;
                }

                Register logical_shift_left(Register a, Register shift_amount)
                {
                        Register result(a.width());

                        for (bigint i = 0; i < a.width(); i++)
                        {
                                bit shifted_bit = bit(0);

                                bigint src_pos = i;
                                for (bigint j = 0; j < shift_amount.width(); j++)
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

                Register logical_shift_right(Register a, Register shift_amount)
                {
                        Register result(a.width());

                        for (bigint i = 0; i < a.width(); i++)
                        {
                                bit shifted_bit = bit(0);

                                bigint src_pos = i;
                                for (bigint j = 0; j < shift_amount.width(); j++)
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

                Register arithmetic_shift_right(Register a, Register shift_amount)
                {
                        Register result(a.width());
                        bit sign_bit = a.at(a.width() - 1);

                        for (bigint i = 0; i < a.width(); i++)
                        {
                                bit shifted_bit = sign_bit;

                                bigint src_pos = i;
                                for (bigint j = 0; j < shift_amount.width(); j++)
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

                // Helper for comparison - no conditionals
                static inline bit bit_vector_compare(const vector<bit> &v, const vector<bit> &w)
                {
                        assert(v.size() == w.size());
                        bit ret = v.at(0) ^ w.at(0);
                        for (bigint i = 1; i < v.size(); i++)
                        {
                                ret |= v.at(i) ^ w.at(i);
                        }
                        return ret;
                }

                Register compare_slt(Register a, Register b)
                {
                        Register result(a.width());
                        Register sub_result(a.width());

                        // Perform subtraction
                        subtract(sub_result, a, b);

                        // Copy sign bit to result[0], all other bits are initialized to 0
                        result.at(0) = sub_result.at(a.width() - 1);
                        return result;
                }

                Register compare_sltu(Register a, Register b)
                {
                        Register result(a.width());
                        bit carry = bit(1); // Start with borrow

                        // Propagate through all bits
                        for (bigint i = 0; i < a.width(); i++)
                        {
                                bit t = (a.at(i) ^ b.at(i));
                                bit next_carry = (a.at(i) & b.at(i)) | (carry & t);
                                carry = next_carry;
                        }

                        // Set result[0] to ~carry, rest are initialized to 0
                        result.at(0) = ~carry;
                        return result;
                }

                // Main ALU function
                Register alu_execute(Register &a, Register &b, vector<bit> alu_op)
                {
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

                        // std::cout << is_add.value() << std::endl;
                        // std::cout << is_sub.value() << std::endl;
                        // std::cout << is_sll.value() << std::endl;
                        // std::cout << is_slt.value() << std::endl;
                        // std::cout << is_sltu.value() << std::endl;
                        // std::cout << is_xor.value() << std::endl;
                        // std::cout << is_srl.value() << std::endl;
                        // std::cout << is_sra.value() << std::endl;
                        // std::cout << is_or.value() << std::endl;
                        // std::cout << is_and.value() << std::endl;

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
                        for (bigint i = 0; i < a.width(); i++)
                        {
                                xor_result.at(i) = a.at(i) ^ b.at(i);
                                or_result.at(i) = a.at(i) | b.at(i);
                                and_result.at(i) = a.at(i) & b.at(i);
                        }

                        for (bigint i = 0; i < result.width(); i++)
                        {
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
        };
};