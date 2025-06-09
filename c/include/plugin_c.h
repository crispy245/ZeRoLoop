// From https://github.com/SpinalHDL/SaxonSoc/blob/dev-0.1/software/standalone/driver/riscv.h
//
// Copyright (c) 2019 SaxonSoc contributors
//
// MIT License: https://github.com/SpinalHDL/SaxonSoc/blob/dev-0.1/LICENSE

#include <stdint.h>

/*
   CFU_OP: Base macro to encode a custom instruction in the CFU style.

   This custom instruction uses the standard R-type RISC-V format:
     [31:25] funct7, [24:20] rs2, [19:15] rs1, [14:12] funct3, [11:7] rd, [6:0] opcode

   The macro parameters are:
     - funct7: 7‐bit immediate (compile‐time constant) for the funct7 field.
     - funct3: 3‐bit immediate (compile‐time constant) for the funct3 field.
     - a:      First operand (will be placed in rs1).
     - b:      Second operand (will be placed in rs2).

   The destination register (rd) is automatically chosen by the compiler
   via the output constraint (“=r”); that is, the result of the instruction
   is stored in a register that becomes the rd field in the encoded instruction.

   The opcode for CFU instructions is assumed to be fixed (here, 0x7B is used as an example).
   Adjust the opcode if your target architecture uses a different one.
*/


asm(".set regnum_x0  ,  0");
asm(".set regnum_x1  ,  1");
asm(".set regnum_x2  ,  2");
asm(".set regnum_x3  ,  3");
asm(".set regnum_x4  ,  4");
asm(".set regnum_x5  ,  5");
asm(".set regnum_x6  ,  6");
asm(".set regnum_x7  ,  7");
asm(".set regnum_x8  ,  8");
asm(".set regnum_x9  ,  9");
asm(".set regnum_x10 , 10");
asm(".set regnum_x11 , 11");
asm(".set regnum_x12 , 12");
asm(".set regnum_x13 , 13");
asm(".set regnum_x14 , 14");
asm(".set regnum_x15 , 15");
asm(".set regnum_x16 , 16");
asm(".set regnum_x17 , 17");
asm(".set regnum_x18 , 18");
asm(".set regnum_x19 , 19");
asm(".set regnum_x20 , 20");
asm(".set regnum_x21 , 21");
asm(".set regnum_x22 , 22");
asm(".set regnum_x23 , 23");
asm(".set regnum_x24 , 24");
asm(".set regnum_x25 , 25");
asm(".set regnum_x26 , 26");
asm(".set regnum_x27 , 27");
asm(".set regnum_x28 , 28");
asm(".set regnum_x29 , 29");
asm(".set regnum_x30 , 30");
asm(".set regnum_x31 , 31");

asm(".set regnum_zero,  0");
asm(".set regnum_ra  ,  1");
asm(".set regnum_sp  ,  2");
asm(".set regnum_gp  ,  3");
asm(".set regnum_tp  ,  4");
asm(".set regnum_t0  ,  5");
asm(".set regnum_t1  ,  6");
asm(".set regnum_t2  ,  7");
asm(".set regnum_s0  ,  8");
asm(".set regnum_s1  ,  9");
asm(".set regnum_a0  , 10");
asm(".set regnum_a1  , 11");
asm(".set regnum_a2  , 12");
asm(".set regnum_a3  , 13");
asm(".set regnum_a4  , 14");
asm(".set regnum_a5  , 15");
asm(".set regnum_a6  , 16");
asm(".set regnum_a7  , 17");
asm(".set regnum_s2  , 18");
asm(".set regnum_s3  , 19");
asm(".set regnum_s4  , 20");
asm(".set regnum_s5  , 21");
asm(".set regnum_s6  , 22");
asm(".set regnum_s7  , 23");
asm(".set regnum_s8  , 24");
asm(".set regnum_s9  , 25");
asm(".set regnum_s10 , 26");
asm(".set regnum_s11 , 27");
asm(".set regnum_t3  , 28");
asm(".set regnum_t4  , 29");
asm(".set regnum_t5  , 30");
asm(".set regnum_t6  , 31");

asm(".set CUSTOM0  , 0x0B");
asm(".set CUSTOM1  , 0x2B");

#ifdef ISSUE_582_WORKAROUND
#define CUSTOM_INSTRUCTION_NOP "nop\n"
#else
#define CUSTOM_INSTRUCTION_NOP
#endif

#define opcode_R(opcode, func3, func7, rs1, rs2)   \
({                                                 \
    register unsigned long result;                 \
    asm volatile(                                  \
     ".word ((" #opcode ") |                       \
     (regnum_%[result] << 7) |                     \
     (regnum_%[arg1] << 15) |                      \
     (regnum_%[arg2] << 20) |                      \
     ((" #func3 ") << 12) |                        \
     ((" #func7 ") << 25));\n"                     \
     CUSTOM_INSTRUCTION_NOP                        \
     : [result] "=r" (result)                      \
     : [arg1] "r" (rs1), [arg2] "r" (rs2)          \
    );                                             \
    result;                                        \
})


// =============== Access the custom instruction

// generic name for each custom instruction - via hardware
#define cfu_op_hw(funct3, funct7, rs1, rs2) \
  opcode_R(CUSTOM0, funct3, funct7, (rs1), (rs2))
#define cfu_op0_hw(funct7, rs1, rs2) cfu_op_hw(0, funct7, rs1, rs2)
#define cfu_op1_hw(funct7, rs1, rs2) cfu_op_hw(1, funct7, rs1, rs2)
#define cfu_op2_hw(funct7, rs1, rs2) cfu_op_hw(2, funct7, rs1, rs2)
#define cfu_op3_hw(funct7, rs1, rs2) cfu_op_hw(3, funct7, rs1, rs2)
#define cfu_op4_hw(funct7, rs1, rs2) cfu_op_hw(4, funct7, rs1, rs2)
#define cfu_op5_hw(funct7, rs1, rs2) cfu_op_hw(5, funct7, rs1, rs2)
#define cfu_op6_hw(funct7, rs1, rs2) cfu_op_hw(6, funct7, rs1, rs2)
#define cfu_op7_hw(funct7, rs1, rs2) cfu_op_hw(7, funct7, rs1, rs2)

#define cfu_op(funct3, funct7, rs1, rs2) cfu_op_hw(funct3, funct7, rs1, rs2)
