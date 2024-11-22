#pragma once

#include "bit.h"
#include <vector>
#include <string>
#include <iostream>

// For loads (lb, lh, lw, lbu, lhu):
#define LOAD_BYTE 0b000   // lb
#define LOAD_HALF 0b001   // lh
#define LOAD_WORD 0b010   // lw
#define LOAD_BYTE_U 0b100 // lbu
#define LOAD_HALF_U 0b101 // lhu

// For stores (sb, sh, sw):
#define STORE_BYTE 0b000 // sb
#define STORE_HALF 0b001 // sh
#define STORE_WORD 0b010 // sw

class Decoder
{
private:
    enum Imm_Len
    {
        I = 12,
        S = 12,
        B = 13,
        U = 20,
        J = 21
    };

    // Helper functions for instruction decoding
    uint32_t get_opcode(uint32_t instruction)
    {
        return instruction & 0x7F; // bits 0-6
    }

    uint32_t get_rd(uint32_t instruction)
    {
        return (instruction >> 7) & 0x1F; // bits 7-11
    }

    uint32_t get_funct3(uint32_t instruction)
    {
        return (instruction >> 12) & 0x7; // bits 12-14
    }

    uint32_t get_rs1(uint32_t instruction)
    {
        return (instruction >> 15) & 0x1F; // bits 15-19
    }

    uint32_t get_rs2(uint32_t instruction)
    {
        return (instruction >> 20) & 0x1F; // bits 20-24
    }

    uint32_t get_funct7(uint32_t instruction)
    {
        return (instruction >> 25) & 0x7F; // bits 25-31
    }

    // Helper to compare ALU ops
    bool compare_alu_ops(const std::vector<bit> &a, const std::vector<bit> &b)
    {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); i++)
        {
            if (a[i].value() != b[i].value())
                return false;
        }
        return true;
    }

    int32_t get_imm_i(uint32_t instruction)
    {
        uint32_t funct3 = get_funct3(instruction);
        int32_t imm;

        // Special case for shift instructions
        if (funct3 == 0b001 || funct3 == 0b101)
        { // SLLI, SRLI, or SRAI
            // For shifts, only use lower 5 bits
            imm = (instruction >> 20) & 0x1F;
        }
        else
        {
            // Normal I-type immediate
            imm = (instruction >> 20) & 0xFFF;
            if (imm & 0x800)
            {
                imm |= 0xFFFFF000; // Sign extend
            }
        }
        return imm;
    }

    int32_t get_imm_s(uint32_t instruction)
    {
        int32_t imm = ((instruction >> 20) & 0xFE0) | // Upper bits
                      ((instruction >> 7) & 0x1F);    // Lower bits
        if (imm & 0x800)
            imm |= 0xFFFFF000; // Sign extend
        return imm;
    }

    int32_t get_imm_b(uint32_t instruction)
    {                                                     // Branch immediate
        int32_t imm = ((instruction >> 31) & 0x1) << 12 | // imm[12]
                      ((instruction >> 7) & 0x1) << 11 |  // imm[11]
                      ((instruction >> 25) & 0x3F) << 5 | // imm[10:5]
                      ((instruction >> 8) & 0xF) << 1;    // imm[4:1]
        // Sign extend
        if (imm & 0x1000)
            imm |= 0xFFFFE000;
        return imm;
    }

    int32_t get_imm_j(uint32_t instruction)
    {                                                      // Jump immediate
        int32_t imm = ((instruction >> 31) & 0x1) << 20 |  // imm[20]
                      ((instruction >> 12) & 0xFF) << 12 | // imm[19:12]
                      ((instruction >> 20) & 0x1) << 11 |  // imm[11]
                      ((instruction >> 21) & 0x3FF) << 1;  // imm[10:1]
        // Sign extend
        if (imm & 0x100000)
            imm |= 0xFFE00000;
        return imm;
    }

public:
    struct DecodedInstruction
    {
        size_t rs1 = 0;
        size_t rs2 = 0;
        size_t rd = 0;
        std::vector<bit> alu_op;
        bool is_alu_op = false;
        bool is_load = false;
        bool is_store = false;
        bool is_immediate = false;
        int32_t imm = 0;
        uint32_t funct3 = 0;
        bool is_branch = false;
        bool is_jump = false;
        bool is_jalr = false;

        bool operator==(const DecodedInstruction &other) const
        {
            return rs1 == other.rs1 &&
                   rs2 == other.rs2 &&
                   rd == other.rd &&
                   compare_alu_op(alu_op, other.alu_op) &&
                   is_alu_op == other.is_alu_op &&
                   is_load == other.is_load &&
                   is_store == other.is_store &&
                   imm == other.imm &&
                   funct3 == other.funct3;
        }

        bool compare_alu_op(const std::vector<bit> &a, const std::vector<bit> &b) const
        {
            if (a.size() != b.size())
                return false;
            for (size_t i = 0; i < a.size(); i++)
            {
                if (a[i].value() != b[i].value())
                    return false;
            }
            return true;
        }
    };
    DecodedInstruction decode(uint32_t instruction)
    {
        DecodedInstruction decoded;
        decoded.alu_op.resize(4, bit(0));

        // Extract fields using existing helpers
        uint32_t opcode = get_opcode(instruction);
        uint32_t funct3 = get_funct3(instruction);
        uint32_t funct7 = get_funct7(instruction);

        // Convert to bit vectors
        vector<bit> op_bits;
        vector<bit> f3_bits;
        vector<bit> f7_bits;

        for (int i = 0; i < 7; i++)
            op_bits.push_back(bit((opcode >> i) & 1));
        for (int i = 0; i < 3; i++)
            f3_bits.push_back(bit((funct3 >> i) & 1));
        for (int i = 0; i < 7; i++)
            f7_bits.push_back(bit((funct7 >> i) & 1));

        // std::cout << "Opcode binary: ";
        // for (int i = 6; i >= 0; i--)
        // {
        //     std::cout << ((opcode >> i) & 1);
        // }
        // std::cout << std::endl;

        // std::cout << "op_bits vector: ";
        // for (int i = 6; i >= 0; i--)
        // {
        //     std::cout << op_bits[i].value();
        // }
        // std::cout << std::endl;

        // Updated instruction type detection (checking all bits)
        bit i_alu = ~op_bits[6] & ~op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0]; // 0010011
        bit r_type = ~op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0]; // 0110011
        bit load = ~op_bits[6] & ~op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0]; // 0000011
        bit store = ~op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0]; // 0100011
        bit branch = op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0]; // 1100011
        bit jal = op_bits[6] & op_bits[5] & ~op_bits[4] & op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];      // 1101111
        bit jalr = op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];   // 1100111

        // R-type operations (following ALU decode pattern)
        bit is_add = r_type & ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0] & ~f7_bits[5];
        bit is_sub = r_type & ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0] & f7_bits[5];
        bit is_sll = r_type & ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
        bit is_slt = r_type & ~f3_bits[2] & f3_bits[1] & ~f3_bits[0];
        bit is_sltu = r_type & ~f3_bits[2] & f3_bits[1] & f3_bits[0];
        bit is_xor = r_type & f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
        bit is_srl = r_type & f3_bits[2] & ~f3_bits[1] & f3_bits[0] & ~f7_bits[5];
        bit is_sra = r_type & f3_bits[2] & ~f3_bits[1] & f3_bits[0] & f7_bits[5];
        bit is_or = r_type & f3_bits[2] & f3_bits[1] & ~f3_bits[0];
        bit is_and = r_type & f3_bits[2] & f3_bits[1] & f3_bits[0];

        // I-type ALU operations
        bit is_addi = i_alu & ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
        bit is_slti = i_alu & ~f3_bits[2] & f3_bits[1] & ~f3_bits[0];
        bit is_sltiu = i_alu & ~f3_bits[2] & f3_bits[1] & f3_bits[0];
        bit is_xori = i_alu & f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
        bit is_ori = i_alu & f3_bits[2] & f3_bits[1] & ~f3_bits[0];
        bit is_andi = i_alu & f3_bits[2] & f3_bits[1] & f3_bits[0];
        bit is_slli = i_alu & ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
        bit is_srli = i_alu & f3_bits[2] & ~f3_bits[1] & f3_bits[0] & ~f7_bits[5];
        bit is_srai = i_alu & f3_bits[2] & ~f3_bits[1] & f3_bits[0] & f7_bits[5];

        // Branch operations
        bit is_beq = branch & ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
        bit is_bne = branch & ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
        bit is_blt = branch & f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
        bit is_bge = branch & f3_bits[2] & ~f3_bits[1] & f3_bits[0];
        bit is_bltu = branch & f3_bits[2] & f3_bits[1] & ~f3_bits[0];
        bit is_bgeu = branch & f3_bits[2] & f3_bits[1] & f3_bits[0];

        vector<bit> alu_op = {bit(0), bit(0), bit(0), bit(0)}; // Default ADD

        vector<bit> sub_op = {bit(1), bit(0), bit(0), bit(0)};
        vector<bit> sll_op = {bit(0), bit(1), bit(0), bit(0)};
        vector<bit> slt_op = {bit(0), bit(1), bit(1), bit(0)};
        vector<bit> sltu_op = {bit(1), bit(1), bit(0), bit(0)};
        vector<bit> xor_op = {bit(1), bit(0), bit(1), bit(0)};
        vector<bit> srl_op = {bit(0), bit(1), bit(1), bit(0)};
        vector<bit> sra_op = {bit(1), bit(1), bit(1), bit(0)};
        vector<bit> or_op = {bit(0), bit(0), bit(0), bit(1)};
        vector<bit> and_op = {bit(1), bit(0), bit(0), bit(1)};

        // Mux operations
        bit_vector_mux(alu_op, sub_op, is_sub | is_beq | is_bne);                // SUB and equality
        bit_vector_mux(alu_op, sll_op, is_sll | is_slli);                        // Shifts left
        bit_vector_mux(alu_op, slt_op, is_slt | is_slti | is_blt | is_bge);      // Signed comparisons
        bit_vector_mux(alu_op, sltu_op, is_sltu | is_sltiu | is_bltu | is_bgeu); // Unsigned
        bit_vector_mux(alu_op, xor_op, is_xor | is_xori);                        // XOR
        bit_vector_mux(alu_op, srl_op, is_srl | is_srli);                        // SRL
        bit_vector_mux(alu_op, sra_op, is_sra | is_srai);                        // SRA
        bit_vector_mux(alu_op, or_op, is_or | is_ori);                           // OR
        bit_vector_mux(alu_op, and_op, is_and | is_andi);                        // AND

        decoded.alu_op = alu_op;

        // Control signals using muxes
        decoded.is_alu_op = 0;
        decoded.is_alu_op = r_type.mux(decoded.is_alu_op, bit(1)).value();
        decoded.is_alu_op = i_alu.mux(decoded.is_alu_op, bit(1)).value();
        decoded.is_alu_op = load.mux(decoded.is_alu_op, bit(1)).value();
        decoded.is_alu_op = store.mux(decoded.is_alu_op, bit(1)).value();
        decoded.is_alu_op = branch.mux(decoded.is_alu_op, bit(1)).value();

        decoded.is_load = load.mux(bit(0), bit(1)).value();
        decoded.is_store = store.mux(bit(0), bit(1)).value();
        decoded.is_branch = branch.mux(bit(0), bit(1)).value();
        decoded.is_jump = 0;
        decoded.is_jump = jal.mux(decoded.is_jump, bit(1)).value();
        decoded.is_jump = jalr.mux(decoded.is_jump, bit(1)).value();
        decoded.is_jalr = jalr.mux(bit(0), bit(1)).value();

        decoded.is_immediate = 0;
        bit is_immediate = bit(0);
        is_immediate = is_immediate | i_alu;
        is_immediate = is_immediate | load;
        is_immediate = is_immediate | store;
        is_immediate = is_immediate | branch;
        is_immediate = is_immediate | jal;
        is_immediate = is_immediate | jalr;
        decoded.is_immediate = is_immediate.value();

        // For debugging
        std::cout << " i_alu: " << i_alu.value() << std::endl;
        std::cout << " load: " << load.value() << std::endl;
        std::cout << " store: " << store.value() << std::endl;
        std::cout << " branch: " << branch.value() << std::endl;
        std::cout << " jal: " << jal.value() << std::endl;
        std::cout << " jalr: " << jalr.value() << std::endl;
        std::cout << " Final is_immediate: " << decoded.is_immediate << std::endl;

        // Register fields
        decoded.rs1 = get_rs1(instruction);
        decoded.rs2 = get_rs2(instruction);
        decoded.rd = get_rd(instruction);

        // Immediate value muxing using existing helper functions
        bit is_i_imm = i_alu | load | jalr;
        bit is_s_imm = store;
        bit is_b_imm = branch;
        bit is_j_imm = jal;

        // Create a 32-bit vector to store our immediate
        std::vector<bit> imm_bits(32, bit(0));

        // For each bit position
        for (int i = 0; i < 32; i++)
        {
            bit curr_bit = bit(0);

            // Create individual bits for each immediate type
            bit i_imm_bit = bit((get_imm_i(instruction) >> i) & 1);
            bit s_imm_bit = bit((get_imm_s(instruction) >> i) & 1);
            bit b_imm_bit = bit((get_imm_b(instruction) >> i) & 1);
            bit j_imm_bit = bit((get_imm_j(instruction) >> i) & 1);

            // Mux each bit individually
            curr_bit = is_i_imm.mux(curr_bit, i_imm_bit);
            curr_bit = is_s_imm.mux(curr_bit, s_imm_bit);
            curr_bit = is_b_imm.mux(curr_bit, b_imm_bit);
            curr_bit = is_j_imm.mux(curr_bit, j_imm_bit);

            imm_bits[i] = curr_bit;
        }

        // Convert bit vector back to integer
        decoded.imm = 0;
        for (int i = 0; i < 32; i++)
        {
            decoded.imm |= (imm_bits[i].value() ? 1 : 0) << i;
        }

        // Store funct3 for load/store width
        decoded.funct3 = funct3;

        return decoded;
    }
};