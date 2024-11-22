#include "zero_loop.h"

ZeroLoop::ZeroLoop(size_t num_registers, size_t reg_width)
    : reg_file(num_registers, reg_width)
{
}

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

void ZeroLoop::execute_instruction(uint32_t instruction)
{
    auto decoded = decoder.decode(instruction);

    if (decoded.is_alu_op)
    {
        Register rs1 = read_register(decoded.rs1);
        Register rs2(32); // Initialize with width

        if (decoded.is_immediate)
        {
            // Handle immediate value
            for (int i = 0; i < 32; i++)
            {
                rs2.at(i) = bit((decoded.imm >> i) & 1);
            }
        }
        else
        {
            rs2 = read_register(decoded.rs2);
        }

        Register result = execute_alu(rs1, rs2, decoded.alu_op);

        // Handle branches
        if (decoded.is_branch)
        {
            int32_t comparison = register_to_int_internal(result);
            bool should_take_branch = false;

            switch (decoded.funct3)
            {
            case 0b000:
                should_take_branch = (comparison == 0);
                break; // BEQ
            case 0b001:
                should_take_branch = (comparison != 0);
                break; // BNE
            case 0b100:
                should_take_branch = (comparison < 0);
                break; // BLT
            case 0b101:
                should_take_branch = (comparison >= 0);
                break; // BGE
            case 0b110:
                should_take_branch = (comparison < 0);
                break; // BLTU
            case 0b111:
                should_take_branch = (comparison >= 0);
                break; // BGEU
            }

            if (should_take_branch)
            {
                pc.update_pc_brj(decoded.imm);
            }
        }
        else if (!decoded.is_store && decoded.rd != 0)
        { // Regular ALU op with destination
            write_register(decoded.rd, result);
        }
    }
    // Memory operations
    else if (decoded.is_load && data_memory != nullptr)
    {
        // Calculate effective address
        Register rs1 = read_register(decoded.rs1);
        int32_t base_addr = register_to_int_internal(rs1);
        int32_t eff_addr = base_addr + decoded.imm;

        std::vector<bit> mem_addr = addr_to_bits(eff_addr >> 2, data_memory->get_addr_bits());
        std::vector<bit> loaded_data = data_memory->read(mem_addr);

        // Handle different load widths based on funct3
        Register loaded_reg(loaded_data);
        write_register(decoded.rd, loaded_reg);
    }
    else if (decoded.is_store && data_memory != nullptr)
    {
        // Calculate effective address
        Register rs1 = read_register(decoded.rs1);
        Register rs2 = read_register(decoded.rs2);
        int32_t base_addr = register_to_int_internal(rs1);
        int32_t eff_addr = base_addr + decoded.imm;

        std::vector<bit> mem_addr = addr_to_bits(eff_addr >> 2, data_memory->get_addr_bits());
        data_memory->write(mem_addr, rs2.get_data());
    }
    else if (decoded.is_jump)
    {
        // Handle JALR
        if (decoded.is_jalr)
        {
            Register rs1_read = read_register(decoded.rs1);
            int32_t rs1_val = register_to_int_internal(rs1_read);
            pc.update_pc_brj((rs1_val + decoded.imm) & ~1);
        }
        // Handle JAL
        else
        {
            pc.increase_pc(decoded.imm);
        }

        // Save return address
        if (decoded.rd != 0)
        {
            Register return_addr(pc.read_pc() + 4, 32);
            write_register(decoded.rd, return_addr);
        }
    }
}

// void ZeroLoop::execute_instruction(uint32_t instruction)
// {

//     //<-----------------DECODE-------------------->
//     auto decoded = decoder.decode(instruction);

//     if (decoded.is_alu_op)
//     {
//         Register rs1 = read_register(decoded.rs1);
//         Register rs2(32); // Initialize with width

//         if (decoded.is_immediate)
//         {
//             // Initialize immediate value register
//             for (int i = 0; i < 32; i++)
//             {
//                 rs2.at(i) = bit((decoded.imm >> i) & 1);
//             }
//         }
//         else
//         {
//             rs2 = read_register(decoded.rs2);
//         }

//         //<-----------------EXECUTE-------------------->
//         Register result = execute_alu(rs1, rs2, decoded.alu_op);
//         int32_t comparison = register_to_int_internal(result);

//         if (decoded.is_branch)
//         {
//             bool should_take_branch = false;
//             switch (decoded.funct3)
//             {
//             case 0b000: // BEQ
//                 should_take_branch = (comparison == 0);
//                 break;
//             case 0b001: // BNE
//                 should_take_branch = (comparison != 0);
//                 break;
//             case 0b100: // BLT
//                 should_take_branch = (comparison != 0);
//                 break;
//             case 0b101: // BGE
//                 should_take_branch = (comparison == 0);
//                 break;
//             case 0b110: // BLTU
//                 should_take_branch = (comparison != 0);
//                 break;
//             case 0b111: // BGEU
//                 should_take_branch = (comparison == 0);
//                 break;
//             }

//             if (should_take_branch)
//             {
//                 pc.update_pc_brj(decoded.imm);
//             }
//         }
//     }
//     else if (decoded.is_jump)
//     {
//         if (decoded.is_jalr)
//         {
//             // JALR: next_pc = (rs1 + imm) & ~1
//             Register rs1_read = read_register(decoded.rs1);
//             int32_t rs1_val = register_to_int_internal(rs1_read);
//             pc.update_pc_brj((rs1_val + decoded.imm) & ~1);
//         }
//         else
//         {
//             // JAL: next_pc = pc + imm
//             pc.increase_pc(decoded.imm);
//         }
//         // Save return address if rd != 0
//         if (decoded.rd != 0)
//         {
//             Register return_addr(pc.read_pc() + 4, 32);
//             write_register(decoded.rd, return_addr);
//         }
//     }
// }

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