#include "zero_loop.h"
#include <iostream>

Register ZeroLoop::read_register(size_t pos)
{
    return reg_file.read(pos);
}

void ZeroLoop::write_register(size_t pos, Register a)
{
    reg_file.write(pos, a);
}

Register ZeroLoop::at_register(size_t index)
{
    return reg_file.at(index);
}

size_t ZeroLoop::get_register_width()
{
    return reg_file.register_width();
}

void ZeroLoop::print_registers()
{
    reg_file.print_all_contents();
}

void ZeroLoop::copy_registers_from(const ZeroLoop &other)
{
    reg_file = other.get_register_file();
}

Register ZeroLoop::execute_alu(Register &a, Register &b, std::vector<bit> alu_op)
{
    return alu.execute(a, b, alu_op);
}

int32_t register_to_int_internal(Register &reg)
{
    int32_t result = 0;
    for (int i = 0; i < reg.width(); i++)
    {
        if (reg.at(i).value())
        {
            result |= (1 << i);
        }
    }
    return result;
}

uint32_t register_to_uint_internal(Register &reg)
{
    uint32_t result = 0;
    for (int i = 0; i < reg.width(); i++)
    {
        if (reg.at(i).value())
        {
            result |= (1U << i);
        }
    }
    return result;
}

// Connect memories
void ZeroLoop::connect_memories(RAM *instr_mem, RAM *data_mem)
{
    instruction_memory = instr_mem;
    data_memory = data_mem;
}

// Helper for memory addressing
std::vector<bit> addr_to_bits(uint32_t addr, size_t bits)
{
    std::vector<bit> result;
    for (size_t i = 0; i < bits; i++)
    {
        result.push_back(bit((addr >> i) & 1));
    }
    return result;
}

void ZeroLoop::full_adder(bit &s, bit &c, bit a, bit b, bit cin)
{
    bit t = (a ^ b);
    s = t ^ cin;
    c = (a & b) | (cin & t);
}

void ZeroLoop::add(Register &ret, Register a, Register b)
{
    bit c;
    for (bigint i = 0; i < a.width(); i++)
    {
        full_adder(ret.at(i), c, a.at(i), b.at(i), c);
    }
}

Register ZeroLoop::two_complement(Register b)
{
    Register complement(b.width());

    // Invert all bits (one's complement)
    for (size_t i = 0; i < b.width(); ++i)
    {
        complement.at(i) = ~b.at(i);
    }

    Register register_holding_1(1, b.width());
    add(complement, complement, register_holding_1);

    return complement;
}

Register ZeroLoop::subtract(Register &result, Register a, Register b)
{
    Register b_complement = two_complement(b);
    bit carry = bit(0);
    add(result, a, b_complement);
    return result;
}

void ZeroLoop::conditional_pc_jump(const bit &should_jump, const Register &target)
{
    if (should_jump.value())
    {
        pc.update_pc_brj(target.get_data_uint());
    }
}

void ZeroLoop::conditional_pc_increment(const bit &should_increment, const Register &offset)
{
    if (should_increment.value())
    {
        pc.increase_pc(offset.get_data_uint());
    }
}

void ZeroLoop::conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, std::vector<bit> f3_bits)
{
    bit is_sb = ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
    bit is_sh = ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
    bit is_sw = ~f3_bits[2] & f3_bits[1] & ~f3_bits[0];

    // This is honestly an ugly hack but we can't just write to partial parts
    // of the ram (at least that am aware of), so load the adress we want,
    // write the parts we modify and send it back
    if (should_write.value() && data_memory != nullptr)
    {
        std::vector<bit> original_data = data_memory->read(addr);
        std::vector<bit> store_data(32);
        for (size_t i = 0; i < 32; i++)
        {
            bit keep_storing = ((is_sb.value() & (i < 8)) || (is_sh.value() & (i < 16)) || (is_sw.value() & (i < 32)));
            store_data.at(i) = keep_storing.mux(original_data.at(i), data.at(i));
        }

        data_memory->write(addr, store_data);
    }
}

void ZeroLoop::conditional_register_write(const bit &should_write, size_t rd, const Register &data)
{
    if (should_write.value() && rd != 0)
    {
        write_register(rd, data);
    }
}

void ZeroLoop::conditional_csr_write(const bit &should_write, size_t csr_pos, const Register &data)
{
    if (should_write.value() && csr_pos != 0)
    {
        csrs[csr_pos] = data;
    }
}

const uint32_t ZeroLoop::get_csr_21()
{
    return csrs[21].get_data_uint();
}

void ZeroLoop::handle_syscall()
{
    std::cout << "PRINT" << std::endl;
    // Get syscall number from a7 (x17)
    Register a7 = read_register(17); // a7 is x17
    Register a0 = read_register(10); // a0 is x10
    std::cout << "DEBUG: Before syscall - PC=0x" << pc.read_pc() << std::endl;
    std::cout << "DEBUG: a0='" << (char)register_to_int_internal(a0) << "'" << std::endl;

    int syscall_num = register_to_int_internal(a7);

    switch (syscall_num)
    {
    case 1: // SYS_PRINT_CHAR
    {
        char c = (char)register_to_int_internal(a0);
        std::cout << c;
        std::cout.flush();
    }
    break;

    case 93: // SYS_EXIT
    {
        int exit_code = register_to_int_internal(a0);
        std::cout << "\nProgram exited with code " << exit_code << std::endl;
    }
    break;
    }
}

void ZeroLoop::execute_instruction(uint32_t instruction)
{
    auto decoded = decoder.decode(instruction);

    // 1. Register File Read Stage
    Register rs1 = read_register(decoded.rs1);
    Register rs2 = read_register(decoded.rs2);

    // 2. ALU Immediate Preparation
    Register rs2_imm(decoded.imm, 32);

    // 3. ALU Input Selection
    Register alu_input_2(32);
    for (int i = 0; i < 32; i++)
    {
        alu_input_2.at(i) = bit(decoded.is_immediate).mux(rs2.at(i), rs2_imm.at(i));
    }

    // 4. Main ALU Operation
    Register alu_result = execute_alu(rs1, alu_input_2, decoded.alu_op);

    // 5. Branch Comparison (separate from main ALU)
    Register branch_comparison(32);
    subtract(branch_comparison, rs1, rs2);

    // 6. Branch Condition Evaluation
    int32_t comparison = register_to_int_internal(branch_comparison);
    uint32_t unsigned_comparison = register_to_uint_internal(branch_comparison);

    bit is_zero(comparison == 0);
    bit is_negative(comparison < 0);
    bit is_greater_unsigned(rs1.get_data_uint() >= rs2.get_data_uint());
    bit is_not_zero = ~is_zero;

    // Calculate branch target
    Register offset(decoded.imm, 32);

    // Branch condition logic
    bit beq_taken = decoded.is_beq & is_zero;
    bit bne_taken = decoded.is_bne & is_not_zero;
    bit blt_taken = decoded.is_blt & is_negative;
    bit bge_taken = decoded.is_bge & ~is_negative;
    bit bltu_taken = decoded.is_bltu & ~is_greater_unsigned;
    bit bgeu_taken = decoded.is_bgeu & is_greater_unsigned;

    std::cout << "BEQ? :" << decoded.is_beq.value() << " is zero? : " << is_zero.value() << std::endl;
    std::cout << "BNE? :" << decoded.is_bne.value() << " is not zero? : " << is_not_zero.value() << std::endl;
    std::cout << "BLT? :" << decoded.is_blt.value() << " is negative? : " << is_negative.value() << std::endl;
    std::cout << "BGE? :" << decoded.is_bge.value() << " is not negative? : " << (!is_negative.value()) << std::endl;
    std::cout << "BLTU? :" << decoded.is_bltu.value() << " is not greater unsigned? : " << (!is_greater_unsigned.value()) << std::endl;
    std::cout << "BGEU? :" << decoded.is_bgeu.value() << " is greater unsigned? : " << is_greater_unsigned.value() << std::endl;

    bit should_branch = (beq_taken | bne_taken | blt_taken | bge_taken | bltu_taken | bgeu_taken) & bit(decoded.is_branch);

    // 7. Memory Address Calculation
    Register mem_base(32);
    Register mem_addr_calc(32);
    add(mem_addr_calc, rs1, offset);
    std::vector<bit> mem_addr = addr_to_bits(mem_addr_calc.get_data_uint(), data_memory->get_addr_bits());

    Register load_result(32);
    if (data_memory != nullptr && decoded.is_load)
    {

        // Decode load type from f3 bits
        bit is_lb = ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        bit is_lh = ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        bit is_lw = ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
        bit is_lbu = decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
        bit is_lhu = decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
        std::vector<bit> loaded_data = data_memory->read(mem_addr);
        std::cout << "DEBUG: Loaded data bits: ";
        for (int i = 7; i >= 0; i--)
        {
            std::cout << loaded_data[i].value();
        }
        std::cout << std::endl;

        bit sign_bit_b = loaded_data[7];  // Sign bit for byte
        bit sign_bit_h = loaded_data[15]; // Sign bit for halfword

        for (size_t i = 0; i < 32; i++)
        {
            bit should_extend_b = (i >= 8) & is_lb.value();
            bit should_extend_h = (i >= 16) & is_lh.value();

            // Determine if this bit position should be loaded
            bit keep_loading = ((is_lb | is_lbu) & (i < 8)) |  // Load byte
                               ((is_lh | is_lhu) & (i < 16)) | // Load halfword
                               (is_lw & (i < 32));             // Load word

            // For sign extension
            bit extended_value = (should_extend_b & sign_bit_b) |
                                 (should_extend_h & sign_bit_h);

            // Select between original data and sign-extended value
            load_result.at(i) = keep_loading.mux(
                extended_value,
                loaded_data[i]);
        }
    }

    bit should_store(decoded.is_store);
    conditional_memory_write(should_store, mem_addr, rs2.get_data(), decoded.f3_bits);

    // 9. Jump Handling
    Register jalr_target(32);
    add(jalr_target, rs1, offset);
    jalr_target.at(0) = bit(0); // Clear least significant bit for JALR

    bit should_jalr = bit(decoded.is_jalr);
    bit should_jal = bit(decoded.jal.value());

    bit is_lui = bit(decoded.lui);
    bit is_auipc = bit(decoded.auipc);

    // 10. PC Update
    Register pc_val(pc.read_pc(), 32);
    Register four(4, 32);
    Register next_pc(32);
    add(next_pc, pc_val, four);

    // Print syscall
    if (instruction == 0x00000073)
    {
        handle_syscall();
        pc.update_pc_brj(next_pc.get_data_uint());
        return;
    }

    Register branch_target(32);
    add(branch_target, pc_val, offset);

    Register jal_target(32);
    add(jal_target, pc_val, rs2_imm);

    Register final_pc(32);

    // AUIPC Update
    Register new_auipc(32);
    add(new_auipc, pc_val, rs2_imm);

    // Default to PC+4
    for (size_t i = 0; i < 32; i++)
    {
        // Default
        final_pc.at(i) = next_pc.at(i);
        // If branch taken
        final_pc.at(i) = should_branch.mux(final_pc.at(i), branch_target.at(i));
        // If JAL taken
        final_pc.at(i) = should_jal.mux(final_pc.at(i), jal_target.at(i));
        // If JALR taken
        final_pc.at(i) = should_jalr.mux(final_pc.at(i), jalr_target.at(i));
    }
    std::cout << "should_branch? " << should_branch.value() << std::endl;
    std::cout << "should_jal? " << should_jal.value() << std::endl;
    std::cout << "should_jalr? " << should_jalr.value() << std::endl;
    jalr_target.print("jalr_target: ");

    // 12. Update PC
    pc.update_pc_brj(final_pc.get_data_uint());

    // 13. Register Write Back
    bit should_write_alu = ~bit(decoded.is_branch) & ~bit(decoded.is_store) & bit(decoded.is_alu_op);
    bit should_write_load = bit(decoded.is_load);
    bit should_write_j = bit(decoded.is_jump);
    bit should_write_jalr = bit(decoded.is_jalr);
    bit should_write_csr = bit(decoded.is_csrrw);

    // Write ALU result, saves alu output on Reg[rd]
    conditional_register_write(should_write_alu, decoded.rd, alu_result);

    // Write load result, saves memory load on Reg[rd]
    conditional_register_write(should_write_load, decoded.rd, load_result);

    // Write return address for JAL/R, saves PC + 4 on Reg[rd]
    conditional_register_write(should_write_j, decoded.rd, next_pc);

    // Write U-Type immediates for LUI, saves U-Imm on Reg[rd]
    conditional_register_write(is_lui, decoded.rd, rs2_imm);

    // Write U-Type immediates for AUIPC, saves PC + U-Imm on Reg[rd]
    conditional_register_write(is_auipc, decoded.rd, new_auipc);

    // Write CSR result
    conditional_register_write(should_write_csr, decoded.rd, csrs[decoded.rs1]);
    conditional_csr_write(should_write_csr, decoded.csr_field, rs1);
    std::cout << "Our csr field is: " << decoded.csr_field << std::endl;

    // Debug output
    std::cout << "Debug Info:" << std::endl;
    std::cout << "rs1 value: " << register_to_int_internal(rs1) << std::endl;
    std::cout << "Branch comparison result: " << comparison << std::endl;
    std::cout << "is_zero: " << is_zero.value() << std::endl;
    std::cout << "is_not_zero: " << is_not_zero.value() << std::endl;
    std::cout << "should_branch: " << should_branch.value() << std::endl;
    std::cout << "Current PC: 0x" << std::hex << pc_val.get_data_uint() << std::dec << std::endl;
    std::cout << "Next PC: 0x" << std::hex << final_pc.get_data_uint() << std::dec << std::endl;
    csrs[21].print("CSR 21 :");
}

RegisterFile &ZeroLoop::get_register_file()
{
    return reg_file;
}

const RegisterFile &ZeroLoop::get_register_file() const
{
    return reg_file;
}

ALU &ZeroLoop::get_alu()
{
    return alu;
}

const ALU &ZeroLoop::get_alu() const
{
    return alu;
}