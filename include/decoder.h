#include "bit.h"
#include <vector>
#include <string>
#include <iostream>

class Decoder {
private:
    // Helper functions for instruction decoding
    uint32_t get_opcode(uint32_t instruction) {
        return instruction & 0x7F;  // bits 0-6
    }
    
    uint32_t get_rd(uint32_t instruction) {
        return (instruction >> 7) & 0x1F;  // bits 7-11
    }
    
    uint32_t get_funct3(uint32_t instruction) {
        return (instruction >> 12) & 0x7;  // bits 12-14
    }
    
    uint32_t get_rs1(uint32_t instruction) {
        return (instruction >> 15) & 0x1F;  // bits 15-19
    }
    
    uint32_t get_rs2(uint32_t instruction) {
        return (instruction >> 20) & 0x1F;  // bits 20-24
    }
    
    uint32_t get_funct7(uint32_t instruction) {
        return (instruction >> 25) & 0x7F;  // bits 25-31
    }
    
    

    // Helper to compare ALU ops
    bool compare_alu_ops(const std::vector<bit>& a, const std::vector<bit>& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); i++) {
            if (a[i].value() != b[i].value()) return false;
        }
        return true;
    }

public:
    struct DecodedInstruction {
        size_t rs1 = 0;
        size_t rs2 = 0;
        size_t rd = 0;
        std::vector<bit> alu_op;
        bool is_alu_op = false;
        
        bool operator==(const DecodedInstruction& other) const {
            return rs1 == other.rs1 && 
                   rs2 == other.rs2 && 
                   rd == other.rd && 
                   compare_alu_op(alu_op, other.alu_op) &&
                   is_alu_op == other.is_alu_op;
        }

        bool compare_alu_op(const std::vector<bit>& a, const std::vector<bit>& b) const {
            if (a.size() != b.size()) return false;
            for (size_t i = 0; i < a.size(); i++) {
                if (a[i].value() != b[i].value()) return false;
            }
            return true;
        }
    };

    DecodedInstruction decode(uint32_t instruction) {
        DecodedInstruction decoded;
        decoded.is_alu_op = false;

        uint32_t opcode = get_opcode(instruction);
        uint32_t funct3 = get_funct3(instruction);
        uint32_t funct7 = get_funct7(instruction);

        // R-type ALU operations
        if (opcode == 0b0110011) {  // R-type
            decoded.is_alu_op = true;
            decoded.rs1 = get_rs1(instruction);
            decoded.rs2 = get_rs2(instruction);
            decoded.rd = get_rd(instruction);

            // Decode ALU operation based on funct3 and funct7
            switch(funct3) {
                case 0b000:  // ADD/SUB
                    if (funct7 == 0b0000000) {
                        // ADD
                        decoded.alu_op = {bit(0), bit(0), bit(0), bit(0)};
                    } else if (funct7 == 0b0100000) {
                        // SUB
                        decoded.alu_op = {bit(1), bit(0), bit(0), bit(0)};
                    }
                    break;
                case 0b001:  // SLL
                    decoded.alu_op = {bit(0), bit(1), bit(0), bit(0)};
                    break;
                case 0b010:  // SLT
                    decoded.alu_op = {bit(0), bit(1), bit(1), bit(0)};
                    break;
                case 0b011:  // SLTU
                    decoded.alu_op = {bit(1), bit(1), bit(0), bit(0)};
                    break;
                case 0b100:  // XOR
                    decoded.alu_op = {bit(1), bit(0), bit(1), bit(0)};
                    break;
                case 0b101:  // SRL/SRA
                    if (funct7 == 0b0000000) {
                        // SRL
                        decoded.alu_op = {bit(0), bit(1), bit(1), bit(0)};
                    } else if (funct7 == 0b0100000) {
                        // SRA
                        decoded.alu_op = {bit(1), bit(1), bit(1), bit(0)};
                    }
                    break;
                case 0b110:  // OR
                    decoded.alu_op = {bit(0), bit(0), bit(0), bit(1)};
                    break;
                case 0b111:  // AND
                    decoded.alu_op = {bit(1), bit(0), bit(0), bit(1)};
                    break;
            }
        }

        return decoded;
    }
};