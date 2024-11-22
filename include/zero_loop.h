#pragma once
#include "register.h"
#include "reg_file.h"
#include "alu.h"
#include "pc.h"
#include "ram_cpu.h"
#include "decoder.h"
#include <vector>

class ZeroLoop {
private:
    RegisterFile reg_file;
    Decoder decoder;
    ALU alu;
    PC pc;
    RAM* instruction_memory; // Pointer to instruction memory
    RAM* data_memory;       // Pointer to data memory

public:
    // Constructor
    ZeroLoop(size_t num_registers = 32, size_t reg_width = 32);

    // RegisterFile operations
    Register read_register(size_t pos);
    void write_register(size_t pos, Register a);
    Register at_register(size_t index);
    size_t get_register_width();
    void print_registers();
    
    // Copy operations
    void copy_registers_from(const ZeroLoop& other);
    
    // ALU operations
    Register execute_alu(Register& a, Register& b, std::vector<bit> alu_op);
    
    // Component access
    RegisterFile& get_register_file();
    const RegisterFile& get_register_file() const;
    ALU& get_alu();
    const ALU& get_alu() const;

    // Stage operations
    void execute_instruction(uint32_t instruction);
    void connect_memories(RAM* instr_mem, RAM* data_mem);
    
};