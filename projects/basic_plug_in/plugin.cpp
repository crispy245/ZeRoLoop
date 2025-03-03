#include "plugin.h"

void full_adder(bit &s, bit &c, bit a, bit b, bit cin)
{
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{

    Register result(0, 32);

    bit c = bit(0);
    for (bigint i = 0; i < a.width(); i++)
    {
        full_adder(result.at(i), c, a.at(i), b.at(i), c);
    }

    // Warning: it's your responsability to wire the correct output depending of the
    // passed on funct3 and funct7
    if(funct3 == 0 & funct7 == 1) return result;
    else return Register(0,32);
}