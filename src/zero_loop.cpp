#include "zero_loop.h"

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

Register ZeroLoop::two_complement(Register b) {
    Register complement(b.width());

    // Invert all bits (one's complement)
    for (size_t i = 0; i < b.width(); ++i) {
        complement.at(i) = ~b.at(i);
    }

    Register register_holding_1(1, b.width());
    add(complement, complement, register_holding_1);

    return complement;
}

Register ZeroLoop::subtract(Register& result, Register a, Register b) {
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

void ZeroLoop::conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data)
{
    if (should_write.value() && data_memory != nullptr)
    {
        data_memory->write(addr, data);
    }
}

void ZeroLoop::conditional_register_write(const bit &should_write, size_t rd, const Register &data)
{
    if (should_write.value() && rd != 0)
    {
        write_register(rd, data);
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
    Register zero(0, 32);
    Register branch_comparison(32);
    subtract(branch_comparison, rs1, zero); // Always compare against zero for branches

    // 6. Branch Condition Evaluation
    int32_t comparison = register_to_int_internal(branch_comparison);
    bit is_zero(comparison == 0);
    bit is_negative(comparison < 0);
    bit is_not_zero = ~is_zero;

    // 7. PC Update Logic
    Register pc_val(pc.read_pc(), 32);
    Register four(4, 32);
    Register next_pc(32);
    add(next_pc, pc_val, four);

    // Calculate branch target
    Register offset(decoded.imm, 32);
    Register branch_target(32);
    add(branch_target, pc_val, offset);

    // Branch condition logic
    bit beq_taken = decoded.is_beq & is_zero;
    bit bne_taken = decoded.is_bne & is_not_zero;
    bit blt_taken = decoded.is_blt & is_negative;
    bit bge_taken = decoded.is_bge & ~is_negative;
    bit bltu_taken = decoded.is_bltu & is_negative;
    bit bgeu_taken = decoded.is_bgeu & ~is_negative;

    bit should_branch = (beq_taken | bne_taken | blt_taken | bge_taken | bltu_taken | bgeu_taken) & bit(decoded.is_branch);

    // 8. Memory Address Calculation
    Register mem_base(32);
    Register mem_addr_calc(32);
    add(mem_addr_calc, rs1, offset);
    std::vector<bit> mem_addr = addr_to_bits(mem_addr_calc.get_data_uint() >> 2, data_memory->get_addr_bits());

    // 9. Memory Operations
    Register load_result(32);
    if (data_memory != nullptr && decoded.is_load)
    {
        std::vector<bit> loaded_data = data_memory->read(mem_addr);
        for (size_t i = 0; i < 32; i++)
        {
            load_result.at(i) = loaded_data[i];
        }
    }

    bit should_store(decoded.is_store);
    conditional_memory_write(should_store, mem_addr, rs2.get_data());

    // 10. Jump Handling
    Register jalr_target(32);
    add(jalr_target, rs1, offset);
    jalr_target.at(0) = bit(0); // Clear least significant bit for JALR

    bit should_jalr = bit(decoded.is_jump) & bit(decoded.is_jalr);
    bit should_jal = bit(decoded.is_jump) & ~bit(decoded.is_jalr);

    // 11. Final PC Selection
    Register final_pc(32);

    // Default to PC+4
    for (size_t i = 0; i < 32; i++)
    {
        final_pc.at(i) = next_pc.at(i);
    }

    // If branch taken, use branch target
    for (size_t i = 0; i < 32; i++)
    {
        final_pc.at(i) = should_branch.mux(final_pc.at(i), branch_target.at(i));
    }

    // If JAL, use JAL target
    for (size_t i = 0; i < 32; i++)
    {
        final_pc.at(i) = should_jal.mux(final_pc.at(i), branch_target.at(i));
    }

    // If JALR, use JALR target
    for (size_t i = 0; i < 32; i++)
    {
        final_pc.at(i) = should_jalr.mux(final_pc.at(i), jalr_target.at(i));
    }

    // 12. Update PC
    pc.update_pc_brj(final_pc.get_data_uint());

    // 13. Register Write Back
    bit should_write_alu = ~bit(decoded.is_branch) & ~bit(decoded.is_store) & bit(decoded.is_alu_op);
    bit should_write_load = bit(decoded.is_load);
    bit should_write_jump = bit(decoded.is_jump);

    // Write ALU result
    conditional_register_write(should_write_alu, decoded.rd, alu_result);

    // Write load result
    conditional_register_write(should_write_load, decoded.rd, load_result);

    // Write return address for jumps
    Register return_addr(next_pc.get_data_uint(), 32);
    conditional_register_write(should_write_jump, decoded.rd, return_addr);

    // Debug output
    std::cout << "Debug Info:" << std::endl;
    std::cout << "rs1 value: " << register_to_int_internal(rs1) << std::endl;
    std::cout << "Branch comparison result: " << comparison << std::endl;
    std::cout << "is_zero: " << is_zero.value() << std::endl;
    std::cout << "is_not_zero: " << is_not_zero.value() << std::endl;
    std::cout << "should_branch: " << should_branch.value() << std::endl;
    std::cout << "Current PC: 0x" << std::hex << pc_val.get_data_uint() << std::dec << std::endl;
    std::cout << "Next PC: 0x" << std::hex << final_pc.get_data_uint() << std::dec << std::endl;
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