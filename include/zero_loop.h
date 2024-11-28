#pragma once
#include "register.h"
#include "reg_file.h"
#include "alu.h"
#include "pc.h"
#include "ram_cpu.h"
#include "decoder.h"
#include <vector>

class ZeroLoop
{
private:
    RegisterFile reg_file;
    Decoder decoder;
    ALU alu;
    PC pc;
    RAM *instruction_memory; // Pointer to instruction memory
    RAM *data_memory;        // Pointer to data memory
    std::vector<Register> csrs;

public:
    // Constructor
    ZeroLoop(size_t num_registers = 32, size_t reg_width = 32)
        : reg_file(num_registers, reg_width),
          alu(),
          pc(0, reg_width),
          instruction_memory(nullptr),
          data_memory(nullptr),
          csrs(4096){}

    // Deep copy constructor
    ZeroLoop(const ZeroLoop& other)
        : reg_file(other.reg_file),
          decoder(other.decoder),
          alu(other.alu),
          pc(other.pc),
          instruction_memory(other.instruction_memory),
          data_memory(other.data_memory),
          csrs(other.csrs) {}

        void copy_state_from(const ZeroLoop& other) {
        reg_file = other.reg_file;
        pc = other.pc;
        instruction_memory = other.instruction_memory;
        data_memory = other.data_memory;
    }

    // RegisterFile operations
    Register read_register(size_t pos);
    void write_register(size_t pos, Register a);
    Register at_register(size_t index);
    size_t get_register_width();
    void print_registers();

    // Copy operations
    void copy_registers_from(const ZeroLoop &other);

    // ALU operations
    Register execute_alu(Register &a, Register &b, std::vector<bit> alu_op);
    Register two_complement(Register b);
    Register subtract(Register& result, Register a, Register b);


    // Component access
    RegisterFile &get_register_file();
    const RegisterFile &get_register_file() const;
    ALU &get_alu();
    const ALU &get_alu() const;
    const uint32_t get_csr_21();

    // Conditional write to units
    void conditional_pc_jump(const bit &should_jump, const Register &target);
    void conditional_pc_increment(const bit &should_increment, const Register &offset);
    void conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data);
    void conditional_register_write(const bit &should_write, size_t rd, const Register &data);
    void conditional_csr_write(const bit &should_write, size_t csr_pos, const Register &data);
 

    void full_adder(bit &s, bit &c, bit a, bit b, bit cin);
    void add(Register &ret, Register a, Register b);
    uint32_t get_pc() { return pc.read_pc();};


    // Stage operations
    void execute_instruction(uint32_t instruction);
    void connect_memories(RAM *instr_mem, RAM *data_mem);
    void run_program();
};