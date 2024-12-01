#pragma once

#include <cassert>
#include "ram.h"
#include "bit_vector.h"
#include "register.h"

using namespace std;

class RegisterFile
{
public:
    RegisterFile(size_t num_registers = 32, size_t width = 32)
        : register_list(num_registers, Register(width))
    {
        // Initialize sp (x2) to __stack_top
        // Converting 0x2400 to decimal (assuming that's where __stack_top is based on your linker script)
        Register stack_ptr(0x2400,32);
        register_list[2] = stack_ptr; // x2 is sp
    }
    RegisterFile(const RegisterFile &other)
        : register_list(other.register_list) {}

    RegisterFile &operator=(const RegisterFile &other)
    {
        if (this != &other)
        { // Prevent self-assignment
            register_list = other.register_list;
        }
        return *this;
    }
    Register at(size_t index);
    void print_all_contents();
    size_t register_width();
    void write(size_t pos, Register a);
    Register read(size_t pos);

private:
    vector<Register> register_list;
};