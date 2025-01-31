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
void ZeroLoop::connect_memories(vector<uint32_t> *instr_mem, RAM *data_mem)
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

Register ZeroLoop::conditional_memory_read(const bit &should_read, const std::vector<bit> &addr, std::vector<bit> f3_bits)
{
    Register result(32);

    if (should_read.value() && data_memory != nullptr)
    {
        // Convert byte address to uint32_t
        uint32_t byte_addr_uint = 0;
        for (size_t i = 0; i < addr.size() && i < 32; ++i)
        {
            if (addr[i].value())
            {
                byte_addr_uint |= (1 << i);
            }
        }
        byte_addr_uint = byte_addr_uint - 8192; // horrible code sorry
        uint32_t word_addr_uint = byte_addr_uint >> 2;
        uint32_t offset = byte_addr_uint & 0x3;

        // Decode funct3
        bool is_lb = (~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0]).value();
        bool is_lh = (~f3_bits[2] & ~f3_bits[1] & f3_bits[0]).value();
        bool is_lw = (~f3_bits[2] & f3_bits[1] & ~f3_bits[0]).value();
        bool is_lbu = (f3_bits[2] & ~f3_bits[1] & ~f3_bits[0]).value();
        bool is_lhu = (f3_bits[2] & ~f3_bits[1] & f3_bits[0]).value();

        // Read primary word
        std::vector<bit> word_addr = addr_to_bits(word_addr_uint, data_memory->get_addr_bits());
        std::vector<bit> word_data = data_memory->read(word_addr);

        // Read next word if needed
        std::vector<bit> next_word_data;
        bool read_next = (is_lw && offset != 0) || ((is_lh || is_lhu) && offset == 3);
        if (read_next)
        {
            uint32_t next_word_addr_uint = word_addr_uint + 1;
            std::vector<bit> next_word_addr = addr_to_bits(next_word_addr_uint, data_memory->get_addr_bits());
            next_word_data = data_memory->read(next_word_addr);
        }

        // Extract relevant bytes
        std::vector<bit> mem_data(32, bit(0));
        if (is_lb || is_lbu)
        {
            // Byte load
            size_t start_bit = offset * 8;
            for (size_t i = 0; i < 8 && (start_bit + i) < word_data.size(); ++i)
            {
                mem_data[i] = word_data[start_bit + i];
            }
            // Sign extend for lb
            if (is_lb)
            {
                bit sign = mem_data[7];
                for (size_t i = 8; i < 32; ++i)
                {
                    mem_data[i] = sign;
                }
            }
        }
        else if (is_lh || is_lhu)
        {
            // Half-word load
            if (offset <= 2)
            {
                size_t start_bit = offset * 8;
                for (size_t i = 0; i < 16 && (start_bit + i) < word_data.size(); ++i)
                {
                    mem_data[i] = word_data[start_bit + i];
                }
            }
            else
            {
                // Combine last byte of current word and first byte of next word
                for (size_t i = 0; i < 8; ++i)
                {
                    mem_data[i] = word_data[24 + i];     // Byte 3 of current word
                    mem_data[i + 8] = next_word_data[i]; // Byte 0 of next word
                }
            }
            // Sign extend for lh
            if (is_lh)
            {
                bit sign = mem_data[15];
                for (size_t i = 16; i < 32; ++i)
                {
                    mem_data[i] = sign;
                }
            }
        }
        else if (is_lw)
        {
            // Word load
            if (offset == 0)
            {
                mem_data = word_data;
            }
            else
            {
                // Combine parts from current and next word
                switch (offset)
                {
                case 1:
                    for (int i = 0; i < 24; ++i)
                        mem_data[i] = word_data[i + 8];
                    for (int i = 0; i < 8; ++i)
                        mem_data[24 + i] = next_word_data[i];
                    break;
                case 2:
                    for (int i = 0; i < 16; ++i)
                        mem_data[i] = word_data[i + 16];
                    for (int i = 0; i < 16; ++i)
                        mem_data[16 + i] = next_word_data[i];
                    break;
                case 3:
                    for (int i = 0; i < 8; ++i)
                        mem_data[i] = word_data[i + 24];
                    for (int i = 0; i < 24; ++i)
                        mem_data[8 + i] = next_word_data[i];
                    break;
                default:
                    break;
                }
            }
        }

        // Assign mem_data to result
        for (int i = 0; i < 32; ++i)
        {
            result.at(i) = mem_data[i];
        }

    }
    return result;
}


void ZeroLoop::conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, std::vector<bit> f3_bits)
{
    bit is_sb = ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
    bit is_sh = ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
    bit is_sw = ~f3_bits[2] & f3_bits[1] & ~f3_bits[0];

    if (should_write.value() && data_memory != nullptr) {
        // Convert byte address to uint32_t
        uint32_t byte_addr_uint = 0;
        for (size_t i = 0; i < addr.size() && i < 32; ++i) {
            if (addr[i].value()) {
                byte_addr_uint |= (1 << i);
            }
        }

        // Adjust for data memory base address (e.g., 0x2000)
        byte_addr_uint = byte_addr_uint - 8192;

        // Calculate word address and byte offset
        uint32_t word_addr_uint = byte_addr_uint >> 2;
        uint32_t offset = byte_addr_uint & 0x3;

        // Convert word address to bits
        std::vector<bit> word_addr = addr_to_bits(word_addr_uint, data_memory->get_addr_bits());
        
        // Read current word from memory
        std::vector<bit> current_word = data_memory->read(word_addr);
        std::vector<bit> next_word;
        bool write_next_word = false;

        // Handle unaligned accesses that cross word boundaries
        if ((is_sh.value() && offset == 3) || (is_sw.value() && offset != 0)) {
            next_word = data_memory->read(addr_to_bits(word_addr_uint + 1, data_memory->get_addr_bits()));
            write_next_word = true;
        }

        // Merge new data into current word
        if (is_sb.value()) {
            // Store byte: replace relevant 8 bits
            size_t start_bit = offset * 8;
            for (size_t i = 0; i < 8; ++i) {
                if (start_bit + i < current_word.size()) {
                    current_word[start_bit + i] = data[i];
                }
            }
        }
        else if (is_sh.value()) {
            // Store halfword: replace relevant 16 bits
            if (offset <= 2) {
                size_t start_bit = offset * 8;
                for (size_t i = 0; i < 16; ++i) {
                    if (start_bit + i < current_word.size()) {
                        current_word[start_bit + i] = data[i];
                    }
                }
            } else {
                // Handle cross-word boundary
                for (size_t i = 0; i < 8; ++i) {
                    current_word[24 + i] = data[i];     // Last byte of current word
                    next_word[i] = data[i + 8];         // First byte of next word
                }
                write_next_word = true;
            }
        }
        else if (is_sw.value()) {
            // Store word
            if (offset == 0) {
                current_word = data;
            } else {
                // Handle unaligned word storage
                size_t bits_in_current = (4 - offset) * 8;
                for (size_t i = 0; i < bits_in_current; ++i) {
                    current_word[offset * 8 + i] = data[i];
                }
                for (size_t i = 0; i < (32 - bits_in_current); ++i) {
                    next_word[i] = data[bits_in_current + i];
                }
                write_next_word = true;
            }
        }

        // Write modified words back to memory
        data_memory->write(word_addr, current_word);
        if (write_next_word) {
            data_memory->write(addr_to_bits(word_addr_uint + 1, data_memory->get_addr_bits()), next_word);
        }
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
    // Get syscall number from a7 (x17)
    Register a7 = read_register(17); // a7 is x17
    Register a0 = read_register(10); // a0 is x10

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

Register sign_extend_offset(int32_t offset, size_t bits)
{
    Register result(32);
    int32_t sign_bit = (offset >> (bits - 1)) & 1; // Extract the sign bit

    // Fill the lower 'bits' with the offset value
    for (size_t i = 0; i < bits; ++i)
    {
        result.at(i) = bit((offset >> i) & 1);
    }
    // Sign extend the higher bits
    for (size_t i = bits; i < 32; ++i)
    {
        result.at(i) = bit(sign_bit);
    }

    return result;
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

    bit should_branch = (beq_taken | bne_taken | blt_taken | bge_taken | bltu_taken | bgeu_taken) & bit(decoded.is_branch);

    Register mem_base(32);
    Register mem_addr_calc(32);
    add(mem_addr_calc, rs1, offset);
    std::vector<bit> mem_addr = mem_addr_calc.get_data();
    // std::vector<bit> mem_addr = addr_to_bits(mem_addr_calc.get_data_uint() >> 2, data_memory->get_addr_bits());

    // TODO WORD ADDRESING IS WRONG! IT SHOULD BE BYTE ADDRESSED MEMORY!
    Register load_result = conditional_memory_read(decoded.is_load, mem_addr, decoded.f3_bits);

    bit should_store(decoded.is_store);

    conditional_memory_write(should_store, mem_addr, rs2.get_data(), decoded.f3_bits);

    // 9. Jump Handling

    // Calculate JALR target as byte address and clear LSB
    Register jalr_target_byte(32);
    add(jalr_target_byte, rs1, offset);
    jalr_target_byte.at(0) = bit(0); // Clear least significant bit for JALR
    // Convert to word address
    uint32_t jalr_target_word_val = jalr_target_byte.get_data_uint() >> 2;
    Register jalr_target_word(jalr_target_word_val, 32);

    bit should_jalr = bit(decoded.is_jalr);
    bit should_jal = bit(decoded.jal.value());

    bit is_lui = bit(decoded.lui);
    bit is_auipc = bit(decoded.auipc);

    // 10. PC Update
    Register pc_val(pc.read_pc(), 32);
    Register pc_val_byte_addr(pc_val.get_data_uint() << 2, 32); // Byte-aligned PC
    Register plus_1(1, 32);
    Register next_pc(32);
    Register next_pc_word(32); // Next PC in word addressing (PC + 1)
    add(next_pc, pc_val, plus_1);
    add(next_pc_word, pc_val, plus_1);

    Register return_addr_byte(32);
    for (int i = 0; i < 32; i++)
    {
        if (i >= 2)
            return_addr_byte.at(i) = next_pc_word.at(i - 2);
        else
            return_addr_byte.at(i) = bit(0);
    }

    // Print syscall
    if (instruction == 0x00000073)
    {
        handle_syscall();
        pc.update_pc_brj(next_pc.get_data_uint());
        return;
    }

    // Calculate Branch targets as byte address
    Register branch_target_byte(32);
    add(branch_target_byte, pc_val_byte_addr, offset);
    // Convert to word address
    uint32_t branch_target_word_val = branch_target_byte.get_data_uint() >> 2;
    Register branch_target_word(branch_target_word_val, 32);

    // Calculate JAL target as byte address
    Register jal_target_byte(32);
    add(jal_target_byte, pc_val_byte_addr, rs2_imm);
    // Convert to word address
    uint32_t jal_target_word_val = jal_target_byte.get_data_uint() >> 2;
    Register jal_target_word(jal_target_word_val, 32);

    Register final_pc(32);

    // AUIPC Update
    // Our Instr. Mem. is Word aligned, therefore when we are at PC=1, in reality this would be address 0x0004
    Register new_auipc(32);
    add(new_auipc, pc_val_byte_addr, rs2_imm);

    // Default to PC+4
    for (size_t i = 0; i < 32; i++)
    {
        // Default
        final_pc.at(i) = next_pc.at(i);
        // If branch taken
        final_pc.at(i) = should_branch.mux(final_pc.at(i), branch_target_word.at(i));
        // If JAL taken
        final_pc.at(i) = should_jal.mux(final_pc.at(i), jal_target_word.at(i));
        // If JALR taken
        final_pc.at(i) = should_jalr.mux(final_pc.at(i), jalr_target_word.at(i));
    }


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
    conditional_register_write(should_write_j, decoded.rd, return_addr_byte);

    // Write U-Type immediates for LUI, saves U-Imm on Reg[rd]

    Register u_type_imm(decoded.imm_unsigned, 32);
    conditional_register_write(is_lui, decoded.rd, u_type_imm);


    // Write U-Type immediates for AUIPC, saves PC + U-Imm on Reg[rd]
    conditional_register_write(is_auipc, decoded.rd, new_auipc);

    // Write CSR result
    conditional_register_write(should_write_csr, decoded.rd, csrs[decoded.rs1]);
    conditional_csr_write(should_write_csr, decoded.csr_field, rs1);

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