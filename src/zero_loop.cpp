#include "zero_loop.h"
#include <stdlib.h>
#include <iomanip>

void ZeroLoop::print_details()
{

    std::cout << "\nTotal Gate Count :" << bit::ops() << " gates" << std::endl;

    std::cout << "\nDetailed Gate Count Breakdown:\n";
    std::cout << "-----------------------------\n";
    for (const auto &op : bit_ops_selectors)
    {
        if (op == bit_ops_cost)
            continue; // Skip the total cost, already printed
        std::cout << std::setw(10) << std::left << bit::opsname(op) << ": "
                  << bit::ops(op) << " gates\n";
    }
}

Register ZeroLoop::read_register(size_t pos)
{
    return reg_file.read(pos);
}

void ZeroLoop::write_register(size_t pos, Register a)
{
    reg_file.write(pos, a);
}

void ZeroLoop::print_registers()
{
    reg_file.print_all_contents();
}

Register ZeroLoop::execute_alu(Register &a, Register &b, std::vector<bit> alu_op)
{
    return alu.execute(a, b, alu_op);
}

Register ZeroLoop::execute_alu_partial(Register &a, Register &b, std::vector<bit> alu_op)
{
    return alu.execute_partial(a, b, alu_op);
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

// Connect memories, overloaded for vector<uint32_t> and RAM
void ZeroLoop::connect_memories(vector<uint32_t> *instr_mem, RAM *data_mem)
{
    instruction_memory_fast = instr_mem;
    data_memory = data_mem;
}

void ZeroLoop::connect_memories(RAM *instr_mem, RAM *data_mem)
{
    instruction_memory_slow = instr_mem;
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

void ZeroLoop::add(Register &ret, Register a, Register b)
{

    alu.add(ret, a, b);
}

void ZeroLoop::subtract(Register &result, Register a, Register b)
{
    alu.subtract(result, a, b);
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
        byte_addr_uint = byte_addr_uint - 8192; // horrible code sorry, but it is supposed to represent where data starts
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

Register ZeroLoop::conditional_memory_read(const bit &should_read, const std::vector<bit> &addr, uint32_t f3_bits)
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
        byte_addr_uint = byte_addr_uint - 8192; // horrible code sorry, but it is supposed to represent where data starts
        uint32_t word_addr_uint = byte_addr_uint >> 2;
        uint32_t offset = byte_addr_uint & 0x3;

        // Decode funct3
        bool is_lb = (f3_bits == 0);  // 000
        bool is_lh = (f3_bits == 1);  // 001
        bool is_lw = (f3_bits == 2);  // 010
        bool is_lbu = (f3_bits == 4); // 100
        bool is_lhu = (f3_bits == 5); // 101

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

// terrible, terrible code, but I don't want to break already working code
// this should be a temporal overload of the function
void ZeroLoop::conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, uint32_t f3_bits)
{

    // Assign bits based on direct comparison
    bit is_sb(f3_bits == 0); // SB: 000
    bit is_sh(f3_bits == 1); // SH: 001
    bit is_sw(f3_bits == 2); // SW: 010

    if (should_write.value() && data_memory != nullptr)
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

        // Adjust for data memory base address
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
        if ((is_sh.value() && offset == 3) || (is_sw.value() && offset != 0))
        {
            next_word = data_memory->read(addr_to_bits(word_addr_uint + 1, data_memory->get_addr_bits()));
            write_next_word = true;
        }

        // Merge new data into current word
        if (is_sb.value())
        {
            // Store byte: replace relevant 8 bits
            size_t start_bit = offset * 8;
            for (size_t i = 0; i < 8; ++i)
            {
                if (start_bit + i < current_word.size())
                {
                    current_word[start_bit + i] = data[i];
                }
            }
        }
        else if (is_sh.value())
        {
            // Store halfword: replace relevant 16 bits
            if (offset <= 2)
            {
                size_t start_bit = offset * 8;
                for (size_t i = 0; i < 16; ++i)
                {
                    if (start_bit + i < current_word.size())
                    {
                        current_word[start_bit + i] = data[i];
                    }
                }
            }
            else
            {
                // Handle cross-word boundary
                for (size_t i = 0; i < 8; ++i)
                {
                    current_word[24 + i] = data[i]; // Last byte of current word
                    next_word[i] = data[i + 8];     // First byte of next word
                }
                write_next_word = true;
            }
        }
        else if (is_sw.value())
        {
            // Store word
            if (offset == 0)
            {
                current_word = data;
            }
            else
            {
                // Handle unaligned word storage
                size_t bits_in_current = (4 - offset) * 8;
                for (size_t i = 0; i < bits_in_current; ++i)
                {
                    current_word[offset * 8 + i] = data[i];
                }
                for (size_t i = 0; i < (32 - bits_in_current); ++i)
                {
                    next_word[i] = data[bits_in_current + i];
                }
                write_next_word = true;
            }
        }

        // Write modified words back to memory
        data_memory->write(word_addr, current_word);
        if (write_next_word)
        {
            data_memory->write(addr_to_bits(word_addr_uint + 1, data_memory->get_addr_bits()), next_word);
        }
    }
}

void ZeroLoop::conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, std::vector<bit> f3_bits)
{
    bit is_sb = ~f3_bits[2] & ~f3_bits[1] & ~f3_bits[0];
    bit is_sh = ~f3_bits[2] & ~f3_bits[1] & f3_bits[0];
    bit is_sw = ~f3_bits[2] & f3_bits[1] & ~f3_bits[0];

    if (should_write.value() && data_memory != nullptr)
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

        // Adjust for data memory base address
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
        if ((is_sh.value() && offset == 3) || (is_sw.value() && offset != 0))
        {
            next_word = data_memory->read(addr_to_bits(word_addr_uint + 1, data_memory->get_addr_bits()));
            write_next_word = true;
        }

        // Merge new data into current word
        if (is_sb.value())
        {
            // Store byte: replace relevant 8 bits
            size_t start_bit = offset * 8;
            for (size_t i = 0; i < 8; ++i)
            {
                if (start_bit + i < current_word.size())
                {
                    current_word[start_bit + i] = data[i];
                }
            }
        }
        else if (is_sh.value())
        {
            // Store halfword: replace relevant 16 bits
            if (offset <= 2)
            {
                size_t start_bit = offset * 8;
                for (size_t i = 0; i < 16; ++i)
                {
                    if (start_bit + i < current_word.size())
                    {
                        current_word[start_bit + i] = data[i];
                    }
                }
            }
            else
            {
                // Handle cross-word boundary
                for (size_t i = 0; i < 8; ++i)
                {
                    current_word[24 + i] = data[i]; // Last byte of current word
                    next_word[i] = data[i + 8];     // First byte of next word
                }
                write_next_word = true;
            }
        }
        else if (is_sw.value())
        {
            // Store word
            if (offset == 0)
            {
                current_word = data;
            }
            else
            {
                // Handle unaligned word storage
                size_t bits_in_current = (4 - offset) * 8;
                for (size_t i = 0; i < bits_in_current; ++i)
                {
                    current_word[offset * 8 + i] = data[i];
                }
                for (size_t i = 0; i < (32 - bits_in_current); ++i)
                {
                    next_word[i] = data[bits_in_current + i];
                }
                write_next_word = true;
            }
        }

        // Write modified words back to memory
        data_memory->write(word_addr, current_word);
        if (write_next_word)
        {
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

void ZeroLoop::conditional_register_write(const bool should_write, size_t rd, const Register &data)
{
    if (should_write && rd != 0)
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
        print_details();
        exit(0);
    }

    case 0: // SYS_EXIT
    {
        int exit_code = register_to_int_internal(a0);
        std::cout << "\nProgram exited with code " << exit_code << std::endl;
        print_details();
        exit(0);
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

void ZeroLoop::execute_instruction_with_decoder_optimized(uint32_t instruction)
{
    auto decoded = decoder.decode(instruction);

    Register rs1 = read_register(decoded.rs1);
    Register rs2 = read_register(decoded.rs2);
    Register rs2_imm(decoded.imm, 32);

    // ALU input selection
    Register alu_input_2(32);
    for (int i = 0; i < 32; i++)
    {
        alu_input_2.at(i) = bit(bit(decoded.is_immediate) & ~decoded.branch).mux(rs2.at(i), rs2_imm.at(i));
    }

    Register alu_result = execute_alu(rs1, alu_input_2, decoded.alu_op);

    bit is_zero = 1; // Assume result is zero
    for (size_t i = 0; i < 32; i++)
    {
        is_zero = is_zero & ~alu_result.at(i);
    }

    bit is_not_zero = ~is_zero;
    bit is_rs1_lesser_rs2 = alu_result.at(0);

    // Branch condition logic
    bit should_branch = (decoded.is_beq & is_zero) | (decoded.is_bne & is_not_zero) |
                        (decoded.is_blt & is_rs1_lesser_rs2) | (decoded.is_bge & ~is_rs1_lesser_rs2) |
                        (decoded.is_bltu & is_rs1_lesser_rs2) | (decoded.is_bgeu & ~is_rs1_lesser_rs2);
    should_branch &= bit(decoded.is_branch);

    // Memory operations
    std::vector<bit> mem_addr = alu_result.get_data();
    Register load_result = conditional_memory_read(decoded.is_load, mem_addr, decoded.f3_bits);
    conditional_memory_write(bit(decoded.is_store), mem_addr, rs2.get_data(), decoded.f3_bits);

    // JALR target calculation
    Register jalr_target_byte(alu_result);
    jalr_target_byte.at(0) = bit(0);
    Register jalr_target_word(jalr_target_byte.get_data_uint() >> 2, 32);

    // PC calculations
    Register pc_val(pc.read_pc(), 32);
    Register pc_val_byte_addr(pc_val.get_data_uint() << 2, 32);
    Register next_pc(pc_val.get_data_uint() + 1, 32);
    Register return_addr_byte(next_pc.get_data_uint() << 2, 32);

    if (instruction == 0x00000073)
    { // Syscall detection
        handle_syscall();
        pc.update_pc_brj(next_pc.get_data_uint());
        return;
    }

    // Branch & Jump targets
    Register branch_target_word((pc_val_byte_addr.get_data_uint() + decoded.imm) >> 2, 32);
    Register jal_target_word(branch_target_word);

    // AUIPC Update
    Register new_auipc(pc_val_byte_addr.get_data_uint() + decoded.imm, 32);
    bit is_auipc = bit(decoded.auipc);

    // Final PC selection
    Register final_pc(32);
    for (size_t i = 0; i < 32; i++)
    {
        final_pc.at(i) = next_pc.at(i);
        final_pc.at(i) = should_branch.mux(final_pc.at(i), branch_target_word.at(i));
        final_pc.at(i) = bit(decoded.jal).mux(final_pc.at(i), jal_target_word.at(i));
        final_pc.at(i) = bit(decoded.is_jalr).mux(final_pc.at(i), jalr_target_word.at(i));
    }

    pc.update_pc_brj(final_pc.get_data_uint());

    // Register Write Back
    conditional_register_write(~bit(decoded.is_branch) & ~bit(decoded.is_store) & bit(decoded.is_alu_op), decoded.rd, alu_result);
    conditional_register_write(bit(decoded.is_load), decoded.rd, load_result);
    conditional_register_write(bit(decoded.is_jump), decoded.rd, return_addr_byte);
    conditional_register_write(bit(decoded.lui), decoded.rd, Register(decoded.imm_unsigned, 32));
    conditional_register_write(is_auipc, decoded.rd, new_auipc);
    conditional_register_write(bit(decoded.is_csrrw), decoded.rd, csrs[decoded.rs1]);
    conditional_csr_write(bit(decoded.is_csrrw), decoded.csr_field, rs1);
}

void ZeroLoop::execute_instruction_with_decoder(uint32_t instruction)
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

    // 10. PC Related
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

    // Detect Syscalls
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
    Register new_auipc(32);
    add(new_auipc, pc_val_byte_addr, rs2_imm);
    bit is_auipc = bit(decoded.auipc);

    // Default to PC+1
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

    // 11. Update PC
    pc.update_pc_brj(final_pc.get_data_uint());

    // 12. Register Write Back
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
    bit is_lui = bit(decoded.lui);
    conditional_register_write(is_lui, decoded.rd, u_type_imm);

    // Write U-Type immediates for AUIPC, saves PC + U-Imm on Reg[rd]
    conditional_register_write(is_auipc, decoded.rd, new_auipc);

    // Write CSR result
    conditional_register_write(should_write_csr, decoded.rd, csrs[decoded.rs1]);
    conditional_csr_write(should_write_csr, decoded.csr_field, rs1);
}

void ZeroLoop::execute_instruction_without_decoder(uint32_t instruction)
{

    // Extract fields directly from instruction
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd_pos = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1_pos = (instruction >> 15) & 0x1F;
    uint32_t rs2_pos = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    // Register read
    Register rs1 = read_register(rs1_pos);
    Register rs2 = read_register(rs2_pos);

    // Immediate extraction (all types)
    int32_t imm_i = (int32_t)(instruction & 0xFFF00000) >> 20;
    int32_t imm_s = ((instruction >> 7) & 0x1F) | ((instruction >> 25) << 5);
    imm_s = (imm_s << 20) >> 20;
    int32_t imm_b = ((instruction >> 7) & 0x1E) |
                    ((instruction >> 20) & 0x7E0) |
                    ((instruction << 4) & 0x800) |
                    ((instruction >> 19) & 0x1000);
    imm_b = (imm_b << 19) >> 19;
    int32_t imm_u = instruction & 0xFFFFF000;
    int32_t imm_j = ((instruction >> 20) & 0x7FE) |
                    ((instruction >> 9) & 0x800) |
                    (instruction & 0xFF000) |
                    ((instruction >> 11) & 0x100000);
    imm_j = (imm_j << 11) >> 11;

    // Select immediate based on opcode
    int32_t imm = 0;
    bool is_load = (opcode == 0x03);
    bool is_store = (opcode == 0x23);
    bool is_branch = (opcode == 0x63);
    bool is_jal = (opcode == 0x6F);
    bool is_jalr = (opcode == 0x67);
    bool is_auipc = (opcode == 0x17);
    bool is_lui = (opcode == 0x37);
    bool is_imm_op = (opcode == 0x13);

    if (is_load || is_jalr || is_imm_op)
        imm = imm_i;
    else if (is_store)
        imm = imm_s;
    else if (is_branch)
        imm = imm_b;
    else if (is_auipc || is_lui)
        imm = imm_u;
    else if (is_jal)
        imm = imm_j;

    Register rs2_imm(imm, 32);

    // Determine instruction type
    bool is_r_type = (opcode == 0x33);
    bool is_i_type = (opcode == 0x13);
    bool is_immediate = is_load || is_store || is_imm_op || is_jalr || is_auipc || is_lui || is_jal;

    // ALU control logic
    std::vector<bit> alu_op(4, bit(0));

    if (is_auipc || is_load || is_store)
    {
        alu_op = {bit(0), bit(0), bit(0), bit(0)}; // ADD: 0000
    }
    else if (funct3 == 0x0)
    { // ADD/SUB
        if (is_r_type && (funct7 & 0x20))
        {
            alu_op = {bit(1), bit(0), bit(0), bit(0)}; // SUB: 1000
        }
        else
        {
            alu_op = {bit(0), bit(0), bit(0), bit(0)}; // ADD: 0000
        }
    }
    else if (funct3 == 0x1)
    {                                              // SLL
        alu_op = {bit(0), bit(0), bit(1), bit(0)}; // 0010
    }
    else if (funct3 == 0x2)
    {                                              // SLT
        alu_op = {bit(0), bit(0), bit(1), bit(1)}; // 0011
    }
    else if (funct3 == 0x3)
    {                                              // SLTU
        alu_op = {bit(0), bit(1), bit(0), bit(0)}; // 0100
    }
    else if (funct3 == 0x4)
    {                                              // XOR
        alu_op = {bit(0), bit(1), bit(0), bit(1)}; // 0101
    }
    else if (funct3 == 0x5)
    { // SRL/SRA
        if (funct7 & 0x20)
        {
            alu_op = {bit(0), bit(1), bit(1), bit(1)}; // SRA: 0111
        }
        else
        {
            alu_op = {bit(0), bit(1), bit(1), bit(0)}; // SRL: 0110
        }
    }
    else if (funct3 == 0x6)
    {                                              // OR
        alu_op = {bit(1), bit(0), bit(0), bit(0)}; // 1000
    }
    else if (funct3 == 0x7)
    {                                              // AND
        alu_op = {bit(1), bit(0), bit(0), bit(1)}; // 1001
    }

    // PC (we increase the PC for Jumps using the ALU)
    // Plus 4 is precalculated every cycle
    Register pc_val(pc.read_pc(), 32);
    Register pc_byte((pc_val.get_data_uint() << 2), 32);
    Register pc_plus_1(pc_val.get_data_uint() + 1, 32);
    Register pc_plus_1_byte(pc_plus_1.get_data_uint() << 2, 32);

    // ALU Input Assignment
    Register alu_input2(32);
    if (is_immediate)
    {
        alu_input2 = rs2_imm;
    }
    else
    {
        alu_input2 = rs2;
    }

    // ALU Execution
    // std::cout << "-------ALU COUNT DISPLAY-------\n";
    // bit::clear_all();
    Register alu_result(0, 32);
    if (instruction == 0x00000073)
    {
        handle_syscall();
    }
    else if (!(is_jal || is_jalr || is_lui))
    {
        alu_result = execute_alu_partial(rs1, alu_input2, alu_op);
    }

    // Memory address calculation
    std::vector<bit> mem_addr_bits = alu_result.get_data();

    // Memory operations (there is some slight overhead here with the shifting)
    int start = bit::ops();
    Register load_result = conditional_memory_read(is_load, mem_addr_bits, funct3);
    conditional_memory_write(is_store, mem_addr_bits, rs2.get_data(), funct3);
    int end = bit::ops();

    // Write back logic
    bool write_alu = !(is_branch || is_store) && (is_imm_op || (opcode == 0x33));
    bool write_load = is_load;
    bool write_jump = is_jal || is_jalr;
    bool write_upper = is_lui || is_auipc;

    Register return_addr((pc_val.get_data_uint() + 1) << 2, 32);
    conditional_register_write(write_jump, rd_pos, return_addr);

    conditional_register_write(write_alu, rd_pos, alu_result);

    conditional_register_write(write_load, rd_pos, load_result);

    if (write_upper)
    {
        if (is_lui)
        {
            conditional_register_write(true, rd_pos, rs2_imm);
        }
        else if (is_auipc)
        {
            conditional_register_write(true, rd_pos, alu_result);
        }
    }

    // Update PC
    //  PC selection logic
    Register final_pc(0, 32);
    if (is_jalr)
    {
        // JALR: PC = (rs1 + imm) & ~1
        uint32_t target_addr = rs1.get_data_uint() + imm;
        // Clear least significant bit as per spec
        target_addr = target_addr & ~1U;
        // Convert back to word address for array indexing
        uint32_t new_pc_word = target_addr >> 2;
        final_pc = Register(new_pc_word, 32);
    }
    else if (is_jal)
    {
        // JAL: PC = PC + imm_j
        uint32_t current_pc_byte = pc_val.get_data_uint() << 2;
        uint32_t target_addr = current_pc_byte + imm;
        // Convert back to word address for array indexing
        uint32_t new_pc_word = target_addr >> 2;
        final_pc = Register(new_pc_word, 32);
    }
    else
    {
        // Normal PC increment
        final_pc = Register(pc_val.get_data_uint() + 1, 32);
    }

    pc.update_pc_brj(final_pc.get_data_uint());

    // print_registers();
}
