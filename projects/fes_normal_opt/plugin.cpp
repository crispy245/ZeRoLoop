#include "plugin.h"
#include "time.h"

Register PLUGIN::execute_plug_in_unit(Register &ret, Register a, Register b,
                                      uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    static unsigned int counter = 0;

    // Seed with time and a counter
    srand(time(NULL) + counter++);

    Register rand_register(rand(), 32);
    if (funct3 == 0 && funct7 == 1)
        return rand_register;
    else
        return Register(0, 32);
}