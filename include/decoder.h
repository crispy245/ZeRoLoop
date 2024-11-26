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
        // Register fields
        size_t rs1 = 0;
        size_t rs2 = 0;
        size_t rd = 0;

        // Control signals as individual bits
        bit i_alu;
        bit r_type;
        bit load;
        bit store;
        bit branch;
        bit jal;
        bit jalr;

        // Instruction type fields
        bool is_alu_op = false;
        bool is_load = false;
        bool is_store = false;
        bool is_immediate = false;
        bool is_branch = false;
        bool is_jump = false;
        bool is_jalr = false;

        // Function bits
        vector<bit> f3_bits; // 3 bits
        vector<bit> f7_bits; // 7 bits
        uint32_t funct3;     // Keep uint32_t version for convenience

        // ALU operation
        std::vector<bit> alu_op;

        // Branch conditions
        bit is_beq;
        bit is_bne;
        bit is_blt;
        bit is_bge;
        bit is_bltu;
        bit is_bgeu;

        // ALU operation flags
        bit is_add;
        bit is_sub;
        bit is_sll;
        bit is_slt;
        bit is_sltu;
        bit is_xor;
        bit is_srl;
        bit is_sra;
        bit is_or;
        bit is_and;

        // I-type ALU flags
        bit is_addi;
        bit is_slti;
        bit is_sltiu;
        bit is_xori;
        bit is_ori;
        bit is_andi;
        bit is_slli;
        bit is_srli;
        bit is_srai;

        // Immediate value and type flags
        int32_t imm;
        bit is_i_imm;
        bit is_s_imm;
        bit is_b_imm;
        bit is_j_imm;

        // Comparison operator
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

        // Initialize bit vectors
        decoded.alu_op.resize(4, bit(0));
        decoded.f3_bits.resize(3, bit(0));
        decoded.f7_bits.resize(7, bit(0));

        // Extract fields using existing helpers
        uint32_t opcode = get_opcode(instruction);
        uint32_t funct3 = get_funct3(instruction);
        uint32_t funct7 = get_funct7(instruction);

        // Store funct3 and funct7 bits
        for (int i = 0; i < 3; i++)
        {
            decoded.f3_bits[i] = bit((funct3 >> i) & 1);
        }
        for (int i = 0; i < 7; i++)
        {
            decoded.f7_bits[i] = bit((funct7 >> i) & 1);
        }
        decoded.funct3 = funct3; // Keep uint32_t version

        // Convert opcode to bit vector
        vector<bit> op_bits;
        for (int i = 0; i < 7; i++)
        {
            op_bits.push_back(bit((opcode >> i) & 1));
        }

        // Instruction type detection
        decoded.i_alu = ~op_bits[6] & ~op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
        decoded.r_type = ~op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
        decoded.load = ~op_bits[6] & ~op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
        decoded.store = ~op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
        decoded.branch = op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
        decoded.jal = op_bits[6] & op_bits[5] & ~op_bits[4] & op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];
        decoded.jalr = op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];

        // R-type operations
        decoded.is_add = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0] & ~decoded.f7_bits[5];
        decoded.is_sub = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0] & decoded.f7_bits[5];
        decoded.is_sll = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_slt = decoded.r_type & ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_sltu = decoded.r_type & ~decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_xor = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_srl = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & ~decoded.f7_bits[5];
        decoded.is_sra = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & decoded.f7_bits[5];
        decoded.is_or = decoded.r_type & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_and = decoded.r_type & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];

        // I-type ALU operations
        decoded.is_addi = decoded.i_alu & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_slti = decoded.i_alu & ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_sltiu = decoded.i_alu & ~decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_xori = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_ori = decoded.i_alu & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_andi = decoded.i_alu & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_slli = decoded.i_alu & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_srli = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & ~decoded.f7_bits[5];
        decoded.is_srai = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & decoded.f7_bits[5];

        // Branch operations
        decoded.is_beq = decoded.branch & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_bne = decoded.branch & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_blt = decoded.branch & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_bge = decoded.branch & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        decoded.is_bltu = decoded.branch & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        decoded.is_bgeu = decoded.branch & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];

        // ALU operation vector setup
        vector<bit> sub_op = {bit(1), bit(0), bit(0), bit(0)};
        vector<bit> sll_op = {bit(0), bit(1), bit(0), bit(0)};
        vector<bit> slt_op = {bit(0), bit(1), bit(1), bit(0)};
        vector<bit> sltu_op = {bit(1), bit(1), bit(0), bit(0)};
        vector<bit> xor_op = {bit(1), bit(0), bit(1), bit(0)};
        vector<bit> srl_op = {bit(0), bit(1), bit(1), bit(0)};
        vector<bit> sra_op = {bit(1), bit(1), bit(1), bit(0)};
        vector<bit> or_op = {bit(0), bit(0), bit(0), bit(1)};
        vector<bit> and_op = {bit(1), bit(0), bit(0), bit(1)};

        // Mux ALU operations
        bit_vector_mux(decoded.alu_op, sub_op, decoded.is_sub | decoded.is_beq | decoded.is_bne);
        bit_vector_mux(decoded.alu_op, sll_op, decoded.is_sll | decoded.is_slli);
        bit_vector_mux(decoded.alu_op, slt_op, decoded.is_slt | decoded.is_slti | decoded.is_blt | decoded.is_bge);
        bit_vector_mux(decoded.alu_op, sltu_op, decoded.is_sltu | decoded.is_sltiu | decoded.is_bltu | decoded.is_bgeu);
        bit_vector_mux(decoded.alu_op, xor_op, decoded.is_xor | decoded.is_xori);
        bit_vector_mux(decoded.alu_op, srl_op, decoded.is_srl | decoded.is_srli);
        bit_vector_mux(decoded.alu_op, sra_op, decoded.is_sra | decoded.is_srai);
        bit_vector_mux(decoded.alu_op, or_op, decoded.is_or | decoded.is_ori);
        bit_vector_mux(decoded.alu_op, and_op, decoded.is_and | decoded.is_andi);

        // Set boolean flags using muxes
        decoded.is_alu_op = decoded.r_type.mux(bit(0), bit(1)).value();
        decoded.is_alu_op = decoded.i_alu.mux(bit(decoded.is_alu_op), bit(1)).value();
        decoded.is_alu_op = decoded.load.mux(bit(decoded.is_alu_op), bit(1)).value();
        decoded.is_alu_op = decoded.store.mux(bit(decoded.is_alu_op), bit(1)).value();
        decoded.is_alu_op = decoded.branch.mux(bit(decoded.is_alu_op), bit(1)).value();

        decoded.is_load = decoded.load.mux(bit(0), bit(1)).value();
        decoded.is_store = decoded.store.mux(bit(0), bit(1)).value();
        decoded.is_branch = decoded.branch.mux(bit(0), bit(1)).value();
        decoded.is_jump = decoded.jal.mux(bit(0), bit(1)).value();
        decoded.is_jump = decoded.jalr.mux(bit(decoded.is_jump), bit(1)).value();
        decoded.is_jalr = decoded.jalr.mux(bit(0), bit(1)).value();

        // Immediate handling
        bit is_immediate = bit(0);
        is_immediate = is_immediate | decoded.i_alu;
        is_immediate = is_immediate | decoded.load;
        is_immediate = is_immediate | decoded.store;
        is_immediate = is_immediate | decoded.branch;
        is_immediate = is_immediate | decoded.jal;
        is_immediate = is_immediate | decoded.jalr;
        decoded.is_immediate = is_immediate.value();

        // Set immediate type flags
        decoded.is_i_imm = decoded.i_alu | decoded.load | decoded.jalr;
        decoded.is_s_imm = decoded.store;
        decoded.is_b_imm = decoded.branch;
        decoded.is_j_imm = decoded.jal;

        // Register fields
        decoded.rs1 = get_rs1(instruction);
        decoded.rs2 = get_rs2(instruction);
        decoded.rd = get_rd(instruction);

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
            curr_bit = decoded.is_i_imm.mux(curr_bit, i_imm_bit);
            curr_bit = decoded.is_s_imm.mux(curr_bit, s_imm_bit);
            curr_bit = decoded.is_b_imm.mux(curr_bit, b_imm_bit);
            curr_bit = decoded.is_j_imm.mux(curr_bit, j_imm_bit);

            imm_bits[i] = curr_bit;
        }

        // Convert bit vector back to integer
        decoded.imm = 0;
        for (int i = 0; i < 32; i++)
        {
            decoded.imm |= (imm_bits[i].value() ? 1 : 0) << i;
        }

        // Debug output
        std::cout << "Instruction type bits:" << std::endl;
        std::cout << " i_alu: " << decoded.i_alu.value() << std::endl;
        std::cout << " load: " << decoded.load.value() << std::endl;
        std::cout << " store: " << decoded.store.value() << std::endl;
        std::cout << " branch: " << decoded.branch.value() << std::endl;
        std::cout << " jal: " << decoded.jal.value() << std::endl;
        std::cout << " jalr: " << decoded.jalr.value() << std::endl;

        std::cout << "Branch condition bits:" << std::endl;
        std::cout << " beq: " << decoded.is_beq.value() << std::endl;
        std::cout << " bne: " << decoded.is_bne.value() << std::endl;
        std::cout << " blt: " << decoded.is_blt.value() << std::endl;
        std::cout << " bge: " << decoded.is_bge.value() << std::endl;
        std::cout << " bltu: " << decoded.is_bltu.value() << std::endl;
        std::cout << " bgeu: " << decoded.is_bgeu.value() << std::endl;

        std::cout << "Function bits:" << std::endl;
        std::cout << " funct3: ";
        for (const auto &b : decoded.f3_bits)
        {
            std::cout << b.value();
        }
        std::cout << std::endl;

        std::cout << "ALU operation bits:" << std::endl;
        std::cout << " alu_op: ";
        for (const auto &b : decoded.alu_op)
        {
            std::cout << b.value();
        }
        std::cout << std::endl;

        std::cout << "Immediate handling:" << std::endl;
        std::cout << " is_immediate: " << decoded.is_immediate << std::endl;
        std::cout << " imm value: " << decoded.imm << std::endl;
        std::cout << " i_imm: " << decoded.is_i_imm.value() << std::endl;
        std::cout << " s_imm: " << decoded.is_s_imm.value() << std::endl;
        std::cout << " b_imm: " << decoded.is_b_imm.value() << std::endl;
        std::cout << " j_imm: " << decoded.is_j_imm.value() << std::endl;

        return decoded;
    }
};