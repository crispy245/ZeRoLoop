#pragma once

#define CUSTOM1 0x2B

#define CUSTOM_I(opcode, funct3, rd, rs1, imm) \
    ( ((imm) << 20) | ((rs1) << 15) | ((funct3) << 12) | ((rd) << 7) | (opcode) )

// Activate counter with counter_id as an immediate value
#define ACTIVATE_COUNTER(counter_id) \
    asm volatile (".word %0" : : "i" (CUSTOM_I(CUSTOM1, 0, 0, 0, counter_id)))

// Deactivate counter with counter_id as an immediate value
#define DEACTIVATE_COUNTER(counter_id) \
    asm volatile (".word %0" : : "i" (CUSTOM_I(CUSTOM1, 1, 0, 0, counter_id)))


