#include "plugin.h"


Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    // Warning: it's your responsability to wire the correct output depending of the
    // passed on funct3 and funct7
    if(funct3 == 0 & funct7 == 1) return Register(0,32);
    else return Register(0,32);
}