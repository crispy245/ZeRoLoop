#pragma once

#include <register.h>

class PLUGIN
{
public:
    Register execute_plug_in_unit(Register &ret, Register a, Register b,
                                  uint32_t funct3, uint32_t funct7, uint32_t opcode);
};
