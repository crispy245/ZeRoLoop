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

// Start = 0, End = 1
// We put the start at the end of ZeroLoop declaration so it does not count
// the overhead of that first instruction and then viceversa with end.
// We put end the the start of ZeroLoop delcation so it... (same idea).
void ZeroLoop::check_for_counter(uint32_t instr, bool start_or_end)
{
    // CUSTOM1 0x2B
    uint32_t opcode = (instr & 0x7F);
    uint32_t funct3 = ((instr >> 12) & 0x7);
    if (opcode == 0x2B) // CUSTOM1
    {
        if (funct3 == 0 && !start_or_end)
        {
            std::cout << "\nCOUNTER0 START" << std::endl;
            start_count1 = bit::ops();
            start_count_only_cpu_1 = total_cpu_gate_count;
        }
        else if (funct3 == 1 && start_or_end)
        {
            end_count1 = bit::ops();
            end_count_only_cpu_1 = total_cpu_gate_count;
            std::cout << "\nCOUNTER0 END" << std::endl;
            std::cout << "TOTAL COUNT OF COUNT0 : " << (end_count1 - start_count1) << " GATES " << std::endl;
            std::cout << "TOTAL COUNT OF COUNT0 (ONLY CPU) : " << (end_count_only_cpu_1 - start_count_only_cpu_1) << " GATES " << std::endl;

        }
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

Register ZeroLoop::execute_plug_in_unit(Register &ret, Register a, Register b, uint32_t funct3, uint32_t funct7, uint32_t opcode)
{
    return plugin.execute_plug_in_unit(ret, a, b, funct3, funct7, opcode);
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
        //std::cout<<"\n byte_addr_unit BEFORE:"<<std::hex<<byte_addr_uint<<std::endl;
        byte_addr_uint = byte_addr_uint - DATA_MEM_BASE; // horrible code sorry, but it is supposed to represent where data starts
        //std::cout<<"\n byte_addr_unit AFTER :"<<std::hex<<byte_addr_uint<<std::endl;

        // if (byte_addr_uint >= DATA_MEM_SIZE*32) {
        //     std::cout << "Memory overflow at: " << std::hex << byte_addr_uint << std::endl;
        //     exit;
        // }
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

        // std::cout<<" Read Next : "<<read_next<<std::endl;
        // std::cout<<" Memory addr :" <<std::hex<<word_addr_uint*4<<std::endl;
        // std::cout<<" Memory read: ";
        // for (int i = static_cast<int>(word_data.size()) - 1; i >= 0; i--)
        // {
        //     std::cout << word_data.at(i).value();
        // }
        // std::cout << std::endl;
        
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
        byte_addr_uint = byte_addr_uint - DATA_MEM_BASE; // horrible code sorry, but it is supposed to represent where data starts
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
        byte_addr_uint = byte_addr_uint - DATA_MEM_BASE;

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
        byte_addr_uint = byte_addr_uint - DATA_MEM_BASE;

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
        std::cout<<"\n The CPU itself (without counting memory interactions) took: "<< total_cpu_gate_count << " gates" << std::endl;
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
    bigint current_instruction_gate_count_start = bit::ops();

    // End counter
    check_for_counter(instruction, 1);

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

    Register plug_in_result(0, 32);
    bigint start_count_mult = bit::ops();
    plug_in_result = execute_plug_in_unit(plug_in_result, rs1, alu_input_2, decoded.funct3, decoded.funct7, decoded.opcode);
    bigint end_count_mult = bit::ops();

    // std::cout<<"MULT TOOK: "<<std::dec<<end_count_mult - start_count_mult<<std::endl;

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

    // We measure up to when memory operations occur. Stop until these are done.
    bigint current_instruction_gate_count_stop = bit::ops();
    total_cpu_gate_count += current_instruction_gate_count_stop - current_instruction_gate_count_start;

    // Memory operations
    std::vector<bit> mem_addr = alu_result.get_data();
    Register load_result = conditional_memory_read(decoded.is_load, mem_addr, decoded.f3_bits);
    if (decoded.is_load)
    {
        // std::cout<<" ADDR IS : "<< alu_result.get_data_uint()<<" LOAD RESULT IS :" <<load_result.get_data_uint()<<std::endl;
    }
    conditional_memory_write(bit(decoded.is_store), mem_addr, rs2.get_data(), decoded.f3_bits);

    current_instruction_gate_count_start = bit::ops();

    // JALR target calculation
    Register jalr_target_byte(alu_result);
    jalr_target_byte.at(0) = bit(0);
    Register jalr_target_word(jalr_target_byte.get_data_uint() >> 2, 32);

    // PC calculations
    Register pc_val(pc.read_pc(), 32);
    Register pc_val_byte_addr(pc_val.get_data_uint() << 2, 32);
    Register next_pc(pc_val.get_data_uint() + 1, 32);
    Register return_addr_byte(next_pc.get_data_uint() << 2, 32);

    // std::cout<<"PC IS : "<<std::hex<<pc_val_byte_addr.get_data_uint()<<std::endl;

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
    conditional_register_write(decoded.custom, decoded.rd, plug_in_result);
    conditional_register_write(~bit(decoded.is_branch) & ~bit(decoded.is_store) & bit(decoded.is_alu_op), decoded.rd, alu_result);
    conditional_register_write(bit(decoded.is_load), decoded.rd, load_result);
    conditional_register_write(bit(decoded.is_jump), decoded.rd, return_addr_byte);
    conditional_register_write(bit(decoded.lui), decoded.rd, Register(decoded.imm_unsigned, 32));
    conditional_register_write(is_auipc, decoded.rd, new_auipc);
    conditional_register_write(bit(decoded.is_csrrw), decoded.rd, csrs[decoded.rs1]);
    conditional_csr_write(bit(decoded.is_csrrw), decoded.csr_field, rs1);

    // Start counter
    check_for_counter(instruction, 0);

    current_instruction_gate_count_stop = bit::ops();
    total_cpu_gate_count += current_instruction_gate_count_stop - current_instruction_gate_count_start;
}


void ZeroLoop::execute_instruction_without_decoder(uint32_t instruction)
{

    bigint current_instruction_gate_count_start = bit::ops();


    check_for_counter(instruction,1);

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
    else if (!(is_jal || is_jalr || is_lui) && opcode != 0X0B)
    {
        alu_result = execute_alu_partial(rs1, alu_input2, alu_op);
    }

    // Plugin interface
    Register plug_in_result(0, 32);
    if (opcode == 0X0B)
    {
        execute_plug_in_unit(plug_in_result, rs1, alu_input2, funct3, funct7, opcode);
    }
    // Memory address calculation
    std::vector<bit> mem_addr_bits = alu_result.get_data();

    bigint current_instruction_gate_count_stop = bit::ops();
    total_cpu_gate_count += current_instruction_gate_count_stop - current_instruction_gate_count_start;
    

    // Memory operations (there is some slight overhead here with the shifting)
    Register load_result = conditional_memory_read(is_load, mem_addr_bits, funct3);
    conditional_memory_write(is_store, mem_addr_bits, rs2.get_data(), funct3);


    current_instruction_gate_count_start = bit::ops();

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

    check_for_counter(instruction,0);


    current_instruction_gate_count_stop = bit::ops();
    total_cpu_gate_count += current_instruction_gate_count_stop - current_instruction_gate_count_start;

}
