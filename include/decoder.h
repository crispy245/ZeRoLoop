#pragma once

#include "bit.h"
#include <vector>
#include <string>
#include <iostream>
#include "bit_vector.h"


class Decoder {
private:
    enum Imm_Len {
        I = 12,
        S = 12,
        B = 13,
        U = 20,
        J = 21
    };

    // Helper functions declarations
    uint32_t get_opcode(uint32_t instruction);
    uint32_t get_rd(uint32_t instruction);
    uint32_t get_funct3(uint32_t instruction);
    uint32_t get_rs1(uint32_t instruction);
    uint32_t get_rs2(uint32_t instruction);
    uint32_t get_funct7(uint32_t instruction);
    bool compare_alu_ops(const std::vector<bit>& a, const std::vector<bit>& b);
    int32_t get_imm_i(uint32_t instruction);
    int32_t get_imm_s(uint32_t instruction);
    int32_t get_imm_b(uint32_t instruction);
    int32_t get_imm_j(uint32_t instruction);
    int32_t get_imm_u(uint32_t instruction);


public:
    struct DecodedInstruction {
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
        bit lui;
        bit auipc;

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

        // CSR 
        uint32_t csr_field;

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
        bit is_u_imm;

        // CSR values and type flags
        bit is_csr_op;
        bit is_csrrw;
        bit is_csrrs;
        bit is_csrrc;
        bit is_csrrwi;
        bit is_csrrsi;
        bit is_csrrci;



        bool operator==(const DecodedInstruction& other) const;
        bool compare_alu_op(const std::vector<bit>& a, const std::vector<bit>& b) const;
    };

    DecodedInstruction decode(uint32_t instruction);
    
};
