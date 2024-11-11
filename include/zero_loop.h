#include "bit_cost.h"
#include "bit_vector_cost.h"
#include "bit_vector.h"
#include "bit.h"
#include "ram.h"

struct ZeRoLoop
{

        struct Register
        {
                vector<bit> data;

                Register(size_t width = 32) : data(width, bit(0)) {}
                Register(vector<bit> &bits) : data(bits) {}
                // TODO Init register with a value, could use it for immediates...

                bit get_bit(size_t index)
                {
                        return data[index];
                }
        };

        struct RegisterFile
        {
                vector<Register> register_list;

                RegisterFile(size_t num_registers = 32, size_t width = 32)
                    : register_list(num_registers, Register(width)) {}
        };

        // Read from register, Handles x0
        static Register register_read(RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits)
        {
                if (H <= L)
                        return Register{};
                if (H == L + 1)
                        return x.register_list.at(L);

                bigint splitpos = 0;
                bigint split = 1;
                while (L + split < H - split)
                {
                        splitpos += 1;
                        split *= 2;
                }
                // now H-L <= split+split
                // and split == 2^splitpos

                // conventional RAM circuit:
                // use i[0:splitpos] to look up entry in x[L:L+split]
                //   result0 = x[L+(I mod split)]
                // use i[0:splitpos] to look up entry in x[L+split:H], called result1
                //   result1 = x[L+split+(I mod split)] if L+split+(I mod split) < H
                // multiplex according to i[splitpos]

                // why this works:
                // assume I < H-L; then I < split+split
                // case 0: i[splitpos] is 0
                //   then I < split
                //   so result0 = x[L+I], and we'll select result0
                // case 1: i[splitpos] is 1
                //   then split <= I < H-L <= split+split
                //   so split+(I mod split) = I <= H-L
                //   so result1 = x[L+I], and we'll select result1

                if (ibits <= splitpos)
                {
                        // different case: definitely want result from x[L:L+split], no multiplexing
                        return register_read(x, L, L + split, i, ibits);
                }

                Register result0 = register_read(x, L, L + split, i, splitpos);
                Register result1 = register_read(x, L + split, H, i, splitpos);

                assert(result0.data.size() == result1.data.size());

                bit isplit = i.data.at(splitpos);

                Register result(0);
                for (bigint r = 0; r < result0.data.size(); ++r)
                {
                        bit x0 = result0.data.at(r);
                        bit x1 = result1.data.at(r);
                        result.data.push_back(isplit.mux(x0, x1));
                }

                return result;
        }
        static Register register_read(
            RegisterFile &x,
            bigint L,
            bigint H,
            Register &i)
        {
                return register_read(x, L, H, i, i.data.size());
        }

        static Register register_read(
            RegisterFile &x,
            Register &i)
        {
                return register_read(x, 0, x.register_list.size(), i);
        }

        static void register_write(
            RegisterFile &x, bigint L, bigint H, Register &i, bigint ibits,
            Register &write_data, bit b = bit(1), bool top = 1) // Added defaults here
        {
                assert(x.register_list[0].data.size() == write_data.data.size());
                if (H <= L)
                        return;
                if (H == L + 1)
                {
                        if (top)
                                x.register_list.at(L) = write_data;
                        else
                                for (bigint r = 0; r < write_data.data.size(); ++r)
                                        x.register_list.at(L).data.at(r) =
                                            b.mux(write_data.data.at(r), x.register_list.at(L).data.at(r));
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
                bit isplit = i.data.at(splitpos);
                register_write(x, L, L + split, i, splitpos, write_data, top ? isplit : (b | isplit), 0);
                register_write(x, L + split, H, i, splitpos, write_data, top ? ~isplit : b.orn(isplit), 0);
        }

        static void register_write(
            RegisterFile &x,
            bigint L,
            bigint H,
            Register &i,
            Register &data)
        {
                register_write(x, L, H, i, i.data.size(), data); // Default b and top will be used
        }

        static void register_write(
            RegisterFile &x,
            Register &i,
            Register &data)
        {
                register_write(x, 0, x.register_list.size(), i, data);
        }

        struct Instruction
        {
                vector<bit> op;
                vector<bit> rs1;
                vector<bit> rs2;
                vector<bit> rd;
        };

        static inline void full_adder(bit &s, bit &c, bit a, bit b, bit c_in)
        {
                s = a ^ b ^ c_in;
                c = (a & b) | (a & c_in) | (b & c_in);
        }
        struct ALU
        {
                static inline void full_adder(bit &s, bit &c, bit a, bit b, bit c_in)
                {
                        s = a ^ b ^ c_in;
                        c = (a & b) | (a & c_in) | (b & c_in);
                }

                static inline void half_adder(bit &s, bit &c, bit a, bit b)
                {
                        s = a ^ b;
                        c = a & b;
                }

                Register alu( Instruction &current_inst, RegisterFile &reg_file)
                {
                        // Get values from registers using rs1 and rs2 as indices

                        Register rs1_idx(current_inst.rs1);
                        Register rs2_idx(current_inst.rs2);

                        Register rs1_value = register_read(reg_file, 0, reg_file.register_list.size(),
                                                           rs1_idx, rs1_idx.data.size());
                        Register rs2_value = register_read(reg_file, 0, reg_file.register_list.size(),
                                                           rs2_idx, rs2_idx.data.size());

                        bit is_add = ~current_inst.op[1] & ~current_inst.op[0];
                        bit is_sub = ~current_inst.op[1] & current_inst.op[0];
                        bit is_nand = current_inst.op[1] & ~current_inst.op[0];
                        bit is_or = current_inst.op[1] & current_inst.op[1];

                        Register add_result(reg_file.register_list.at(0).data.size());
                        bit c(0);
                        for (bigint i = 0; i < rs2_value.data.size(); i++)
                                full_adder(add_result.data.at(i), c, rs1_value.data.at(i), rs2_value.data.at(i), 0);

                        Register sub_result(reg_file.register_list.at(0).data.size());
                        for (bigint i = rs2_value.data.size(); i < rs1_value.data.size(); i++)
                                half_adder(sub_result.data.at(i), c, rs1_value.data.at(i), c);

                        Register nand_result(reg_file.register_list.at(0).data.size());
                        for (bigint i = 0; i < rs2_value.data.size(); i++)
                                nand_result.data.at(i) = ~(rs1_value.data.at(i) & rs2_value.data.at(i));

                        Register or_result(reg_file.register_list.at(0).data.size());
                        for (bigint i = 0; i < rs2_value.data.size(); i++)
                                or_result.data.at(i) = (rs1_value.data.at(i) | rs2_value.data.at(i));

                        Register result(reg_file.register_list.at(0).data.size());
                        for (bigint i = 0; i < result.data.size(); i++)
                        {
                                // Level 1 Muxes
                                bit temp1 = is_add.mux(add_result.data[i], sub_result.data[i]);
                                bit temp2 = is_nand.mux(nand_result.data[i], or_result.data[i]);

                                // Level 2 Muxes
                                result.data[i] = (~current_inst.op[1]).mux(temp1, temp2);
                        }

                        return result;
                }
        };
};
