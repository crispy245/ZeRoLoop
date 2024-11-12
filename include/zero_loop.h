#include "bit_cost.h"
#include "bit_vector_cost.h"
#include "bit_vector.h"
#include "bit.h"
#include "ram.h"
#include <iostream>

struct ZeRoLoop {
    struct Register {
        vector<bit> data;

        Register(size_t width = 32) : data(width, bit(0)) {}
        Register(vector<bit> &bits) : data(bits) {}

        bit get_bit(size_t index) const {
            return data[index];
        }

        // Get number of bits in register
        size_t width() const {
            return data.size();
        }

        // Print register contents
        void print(const std::string &name) const {
            std::cout << name << ": ";
            for (int i = width() - 1; i >= 0; i--) {
                std::cout << data[i].value();
            }
            std::cout << std::endl;
        }

        // Access individual bits with bounds checking
        bit& at(size_t index) {
            return data.at(index);
        }

        const bit& at(size_t index) const {
            return data.at(index);
        }

        void push_back(const bit& b) {
            data.push_back(b);
        }
    };

    struct RegisterFile {
        vector<Register> register_list;

        RegisterFile(size_t num_registers = 32, size_t width = 32)
            : register_list(num_registers, Register(width)) {}

        // Access registers with bounds checking
        Register& at(size_t index) {
            return register_list.at(index);
        }

        const Register& at(size_t index) const {
            return register_list.at(index);
        }

        size_t count() const {
            return register_list.size();
        }
    };

    static Register register_read(RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits) {
        if (H <= L)
            return Register{};
        if (H == L + 1)
            return x.at(L);

        bigint splitpos = 0;
        bigint split = 1;
        while (L + split < H - split) {
            splitpos += 1;
            split *= 2;
        }

        if (ibits <= splitpos) {
            return register_read(x, L, L + split, i, ibits);
        }

        Register result0 = register_read(x, L, L + split, i, splitpos);
        Register result1 = register_read(x, L + split, H, i, splitpos);

        assert(result0.width() == result1.width());

        bit isplit = i.at(splitpos);

        Register result(0);
        for (bigint r = 0; r < result0.width(); ++r) {
            bit x0 = result0.at(r);
            bit x1 = result1.at(r);
            result.push_back(isplit.mux(x0, x1));
        }

        return result;
    }

    static Register register_read(RegisterFile &x, bigint L, bigint H, Register &i) {
        return register_read(x, L, H, i, i.width());
    }

    static Register register_read(RegisterFile &x, Register &i) {
        return register_read(x, 0, x.count(), i);
    }

    static void register_write(RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits,
                             Register &write_data, bit b = bit(1), bool top = 1) {
        assert(x.at(0).width() == write_data.width());
        if (H <= L)
            return;
        if (H == L + 1) {
            if (top)
                x.at(L) = write_data;
            else
                for (bigint r = 0; r < write_data.width(); ++r)
                    x.at(L).at(r) = b.mux(write_data.at(r), x.at(L).at(r));
            return;
        }

        bigint splitpos = 0;
        bigint split = 1;
        while (L + split < H - split) {
            splitpos += 1;
            split *= 2;
        }

        if (ibits <= splitpos) {
            return register_write(x, L, L + split, i, ibits, write_data, b, top);
        }

        bit isplit = i.at(splitpos);
        register_write(x, L, L + split, i, splitpos, write_data, top ? isplit : (b | isplit), 0);
        register_write(x, L + split, H, i, splitpos, write_data, top ? ~isplit : b.orn(isplit), 0);
    }

    static void register_write(RegisterFile &x, bigint L, bigint H, Register &i, Register &data) {
        register_write(x, L, H, i, i.width(), data);
    }

    static void register_write(RegisterFile &x, Register &i, Register &data) {
        register_write(x, 0, x.count(), i, data);
    }

    struct Instruction {
        vector<bit> op;
        vector<bit> rs1;
        vector<bit> rs2;
        vector<bit> rd;
    };

    struct ALU {
        static inline void half_adder(bit &s, bit &c, bit a, bit b) {
            s = a ^ b;
            c = a & b;
        }

        static inline void full_adder(bit &s, bit &c, bit a, bit b) {
            bit t = (a ^ b);
            s = t ^ c;
            c = (a & b) | (c & t);
        }

        Register register_add(Register &ret, Register a, Register b, bit c_in) {
            bit c = c_in;
            for (bigint i = 0; i < b.width(); i++)
                full_adder(ret.at(i), c, a.at(i), b.at(i));

            for (bigint i = b.width(); i < a.width(); i++)
                half_adder(ret.at(i), c, a.at(i), c);

            if (a.width() < ret.width())
                ret.at(a.width()) = c;

            return ret;
        }

        Register register_negate(Register &ret, Register a) {
            for (bigint i = 0; i < a.width(); i++)
                ret.at(i) = ~a.at(i);
            return ret;
        }

        Register alu(Instruction &current_inst, RegisterFile &reg_file) {
            Register rs1_idx(current_inst.rs1);
            Register rs2_idx(current_inst.rs2);
            Register rs1_value = register_read(reg_file, 0, reg_file.count(), rs1_idx, rs1_idx.width());
            Register rs2_value = register_read(reg_file, 0, reg_file.count(), rs2_idx, rs2_idx.width());

            bit is_add = ~current_inst.op[1] & ~current_inst.op[0];
            bit is_sub = ~current_inst.op[1] & current_inst.op[0];
            bit is_nand = current_inst.op[1] & ~current_inst.op[0];
            bit is_or = current_inst.op[1] & current_inst.op[0];

            Register add_result(rs1_value.width());
            register_add(add_result, rs1_value, rs2_value, 0);

            Register sub_result(rs1_value.width());
            Register negated_rs2(rs1_value.width());
            register_negate(negated_rs2, rs2_value);
            register_add(sub_result, rs1_value, negated_rs2, 1);

            Register result(reg_file.at(0).width());
            for (bigint i = 0; i < result.width(); i++) {
                bit temp1 = is_add.mux(add_result.at(i), sub_result.at(i));
                bit temp2 = is_nand.mux(add_result.at(i), sub_result.at(i));
                result.at(i) = (~current_inst.op[1]).mux(temp1, temp2);
            }

            return result;
        }
    };
};