#include "pc.h"

void PC::full_adder(bit& s, bit& c, bit a, bit b, bit cin) {
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

void PC::add(Register& ret, Register a, Register b) {
    bit c;
    for (bigint i = 0; i < a.width(); i++) {
        full_adder(ret.at(i), c, a.at(i), b.at(i), c);
    }
}

PC::PC(bigint start_pc, size_t reg_width) 
    : current_pc(start_pc, reg_width) {}

PC& PC::operator=(const PC& new_pc) {
    if (this != &new_pc) {
        current_pc = new_pc.current_pc;
    }
    return *this;
}

void PC::increase_pc(bigint offset_amount) {
    assert((offset_amount % 4) == 0);
    Register offset_amount_register(offset_amount, current_pc.width());
    add(current_pc, current_pc, offset_amount_register);
}

void PC::increase_pc_add_four_too(bigint offset_amount) {
    assert((offset_amount % 4) == 0);
    bigint offset_amount_plus_4 = offset_amount + 4;
    Register offset_amount_register(offset_amount_plus_4, current_pc.width());
    add(current_pc, current_pc, offset_amount_register);
}

void PC::update_pc_brj(bigint new_pc_val) {
    current_pc.update_data(new_pc_val);
}

uint32_t PC::read_pc() {
    return current_pc.get_data_uint();
}