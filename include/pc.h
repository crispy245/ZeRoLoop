#pragma once

#include "register.h"
#include "vector"
#include "alu.h"
class PC
{
private:
    Register current_pc;

    void full_adder(bit &s, bit &c, bit a, bit b, bit cin)
    {
        bit t = (a ^ b);
        s = t ^ cin;
        c = (a & b) | (cin & t);
    }

    void add(Register &ret, Register a, Register b)
    {
        bit c;
        for (bigint i = 0; i < a.width(); i++)
        {
            full_adder(ret.at(i), c, a.at(i), b.at(i), c);
        }
    }

public:
    PC(bigint start_pc = 0, size_t reg_width = 32)
        : current_pc(start_pc, reg_width) {}

    PC &operator=(const PC &new_pc)
    {
        if (this != &new_pc)
        {
            current_pc = new_pc.current_pc;
        }
        return *this;
    }

    // Use for normal PC counter updates
    void increase_pc(bigint offset_amount)
    {
        assert((offset_amount % 4) == 0);
        Register offset_amount_register(offset_amount, current_pc.width());
        add(current_pc, current_pc, offset_amount_register);
    }

    // Use for branches and jumps
    void update_pc_brj(bigint new_pc_val)
    {
        assert((new_pc_val % 4) == 0);
        current_pc.update_data(new_pc_val);
    }

    uint32_t read_pc(){
        return current_pc.get_data_uint();
    }

    
};