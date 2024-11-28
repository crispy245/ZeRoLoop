#pragma once
#include "register.h"
#include "alu.h"
#include <vector>
#include <cstdint>
#include <cassert>

class PC {
private:
    Register current_pc;
    
    // Helper function declarations
    void full_adder(bit& s, bit& c, bit a, bit b, bit cin);
    void add(Register& ret, Register a, Register b);

public:
    // Constructor
    PC(bigint start_pc = 0, size_t reg_width = 32);
    
    // Assignment operator
    PC& operator=(const PC& new_pc);
    
    // PC update methods
    void increase_pc(bigint offset_amount);
    void update_pc_brj(bigint new_pc_val);
    uint32_t read_pc();
};