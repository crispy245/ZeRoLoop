#pragma once
#include "register.h"
#include "reg_file.h"
#include "alu.h"
#include <vector>
class ZeroLoop
{
private:
    RegisterFile reg_file;
    ALU alu;

public:
    // Constructor
    ZeroLoop(size_t num_registers = 32, size_t reg_width = 32)
        : reg_file(num_registers, reg_width) {}

    // RegisterFile wrapper functions
    Register read_register(size_t pos)
    {
        return reg_file.read(pos);
    }

    void write_register(size_t pos, Register a)
    {
        reg_file.write(pos, a);
    }

    Register at_register(size_t index)
    {
        return reg_file.at(index);
    }

    size_t get_register_width()
    {
        return reg_file.register_width();
    }

    void print_registers()
    {
        reg_file.print_all_contents();
    }

    // Copy registers from another ZeroLoop
    void copy_registers_from(const ZeroLoop &other)
    {
        reg_file = other.get_register_file();
    }

    // ALU operations
    Register execute_alu(Register &a, Register &b, std::vector<bit> alu_op)
    {
        return alu.execute(a, b, alu_op);
    }

    // Access to underlying components if needed
    RegisterFile &get_register_file() { return reg_file; }
    const RegisterFile &get_register_file() const { return reg_file; }
    ALU &get_alu() { return alu; }
    const ALU &get_alu() const { return alu; }
};