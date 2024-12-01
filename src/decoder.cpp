#include "decoder.h"

uint32_t Decoder::get_opcode(uint32_t instruction)
{
    return instruction & 0x7F; // bits 0-6
}

uint32_t Decoder::get_rd(uint32_t instruction)
{
    return (instruction >> 7) & 0x1F; // bits 7-11
}

uint32_t Decoder::get_funct3(uint32_t instruction)
{
    return (instruction >> 12) & 0x7; // bits 12-14
}

uint32_t Decoder::get_rs1(uint32_t instruction)
{
    return (instruction >> 15) & 0x1F; // bits 15-19
}

uint32_t Decoder::get_rs2(uint32_t instruction)
{
    return (instruction >> 20) & 0x1F; // bits 20-24
}

uint32_t Decoder::get_funct7(uint32_t instruction)
{
    return (instruction >> 25) & 0x7F; // bits 25-31
}

bool Decoder::compare_alu_ops(const std::vector<bit> &a, const std::vector<bit> &b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
    {
        if (a[i].value() != b[i].value())
            return false;
    }
    return true;
}

int32_t Decoder::get_imm_i(uint32_t instruction)
{
    uint32_t funct3 = get_funct3(instruction);
    int32_t imm;

    if (funct3 == 0b001 || funct3 == 0b101)
    { // SLLI, SRLI, or SRAI
        imm = (instruction >> 20) & 0x1F;
    }
    else
    {
        imm = (instruction >> 20) & 0xFFF;
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000; // Sign extend
        }
    }
    return imm;
}

int32_t Decoder::get_imm_s(uint32_t instruction)
{
    int32_t imm = ((instruction >> 20) & 0xFE0) | // Upper bits
                  ((instruction >> 7) & 0x1F);    // Lower bits
    if (imm & 0x800)
        imm |= 0xFFFFF000; // Sign extend
    return imm;
}

int32_t Decoder::get_imm_b(uint32_t instruction)
{
    int32_t imm = ((instruction >> 31) & 0x1) << 12 | // imm[12]
                  ((instruction >> 7) & 0x1) << 11 |  // imm[11]
                  ((instruction >> 25) & 0x3F) << 5 | // imm[10:5]
                  ((instruction >> 8) & 0xF) << 1;    // imm[4:1]
    if (imm & 0x1000)
        imm |= 0xFFFFE000;
    return imm;
}

int32_t Decoder::get_imm_j(uint32_t instruction)
{
    int32_t imm = ((instruction >> 31) & 0x1) << 20 |  // imm[20]
                  ((instruction >> 12) & 0xFF) << 12 | // imm[19:12]
                  ((instruction >> 20) & 0x1) << 11 |  // imm[11]
                  ((instruction >> 21) & 0x3FF) << 1;  // imm[10:1]
    if (imm & 0x100000)
        imm |= 0xFFE00000;
    return imm;
}
int32_t Decoder::get_imm_u(uint32_t instruction)
{
    return (instruction & 0xFFFFF000); // imm[31:12]
}

bool Decoder::DecodedInstruction::operator==(const DecodedInstruction &other) const
{
    return rs1 == other.rs1 &&
           rs2 == other.rs2 &&
           rd == other.rd &&
           compare_alu_op(alu_op, other.alu_op) &&
           is_alu_op == other.is_alu_op &&
           is_load == other.is_load &&
           is_store == other.is_store &&
           imm == other.imm &&
           funct3 == other.funct3;
}

bool Decoder::DecodedInstruction::compare_alu_op(const std::vector<bit> &a, const std::vector<bit> &b) const
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
    {
        if (a[i].value() != b[i].value())
            return false;
    }
    return true;
}

Decoder::DecodedInstruction Decoder::decode(uint32_t instruction)
{
    DecodedInstruction decoded;

    // Initialize bit vectors
    decoded.alu_op.resize(4, bit(0));
    decoded.f3_bits.resize(3, bit(0));
    decoded.f7_bits.resize(7, bit(0));

    // Extract fields using existing helpers
    uint32_t opcode = get_opcode(instruction);
    uint32_t funct3 = get_funct3(instruction);
    uint32_t funct7 = get_funct7(instruction);

    // Store funct3 and funct7 bits
    std::cout<<"funct3 : "<<funct3<<" ";
    for (int i = 0; i < 3; i++)
    {
        decoded.f3_bits[i] = bit((funct3 >> i) & 1);
    }
    std::cout<<"after shift..."<<std::endl;
    for(auto x : decoded.f3_bits){
        std::cout<<x.value()<<" ";
    }
    std::cout<<endl;

    for (int i = 0; i < 7; i++)
    {
        decoded.f7_bits[i] = bit((funct7 >> i) & 1);
    }
    decoded.funct3 = funct3; // Keep uint32_t version

    // Convert opcode to bit vector
    std::vector<bit> op_bits;
    for (int i = 0; i < 7; i++)
    {
        op_bits.push_back(bit((opcode >> i) & 1));
    }
    std::cout << "Opcode bits: ";
    for (int i = 6; i >= 0; i--)
    {
        std::cout << op_bits[i].value();
    }
    std::cout << std::endl;

    // Instruction type detection
    decoded.i_alu = ~op_bits[6] & ~op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
    decoded.r_type = ~op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
    decoded.load = ~op_bits[6] & ~op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
    decoded.store = ~op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
    decoded.branch = op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];
    decoded.jal = op_bits[6] & op_bits[5] & ~op_bits[4] & op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];
    decoded.jalr = op_bits[6] & op_bits[5] & ~op_bits[4] & ~op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];
    decoded.lui = ~op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];
    decoded.auipc = ~op_bits[6] & ~op_bits[5] & op_bits[4] & ~op_bits[3] & op_bits[2] & op_bits[1] & op_bits[0];

    // R-type operations
    decoded.is_add = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0] & ~decoded.f7_bits[5];
    decoded.is_sub = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0] & decoded.f7_bits[5];
    decoded.is_sll = decoded.r_type & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_slt = decoded.r_type & ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_sltu = decoded.r_type & ~decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_xor = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_srl = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & ~decoded.f7_bits[5];
    decoded.is_sra = decoded.r_type & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & decoded.f7_bits[5];
    decoded.is_or = decoded.r_type & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_and = decoded.r_type & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];

    // I-type ALU operations
    decoded.is_addi = decoded.i_alu & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_slti = decoded.i_alu & ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_sltiu = decoded.i_alu & ~decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_xori = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_ori = decoded.i_alu & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_andi = decoded.i_alu & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_slli = decoded.i_alu & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_srli = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & ~decoded.f7_bits[5];
    decoded.is_srai = decoded.i_alu & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0] & decoded.f7_bits[5];

    // Branch operations
    decoded.is_beq = decoded.branch & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_bne = decoded.branch & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_blt = decoded.branch & decoded.f3_bits[2] & ~decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_bge = decoded.branch & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_bltu = decoded.branch & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_bgeu = decoded.branch & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];

    // CSR operations

    decoded.is_csr_op = op_bits[6] & op_bits[5] & op_bits[4] & ~op_bits[3] & ~op_bits[2] & op_bits[1] & op_bits[0];

    decoded.is_csrrw = decoded.is_csr_op & ~decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_csrrs = decoded.is_csr_op & ~decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_csrrc = decoded.is_csr_op & ~decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_csrrwi = decoded.is_csr_op & decoded.f3_bits[2] & ~decoded.f3_bits[1] & decoded.f3_bits[0];
    decoded.is_csrrsi = decoded.is_csr_op & decoded.f3_bits[2] & decoded.f3_bits[1] & ~decoded.f3_bits[0];
    decoded.is_csrrci = decoded.is_csr_op & decoded.f3_bits[2] & decoded.f3_bits[1] & decoded.f3_bits[0];

    // CSR
    decoded.csr_field = (instruction >> 20) & 0xFFF; // CSR field is in bits 31:20

    // ALU operation vector setup
    std::vector<bit> sub_op = {bit(1), bit(0), bit(0), bit(0)};
    std::vector<bit> sll_op = {bit(0), bit(1), bit(0), bit(0)};
    std::vector<bit> slt_op = {bit(0), bit(1), bit(1), bit(0)};
    std::vector<bit> sltu_op = {bit(1), bit(1), bit(0), bit(0)};
    std::vector<bit> xor_op = {bit(1), bit(0), bit(1), bit(0)};
    std::vector<bit> srl_op = {bit(0), bit(1), bit(1), bit(0)};
    std::vector<bit> sra_op = {bit(1), bit(1), bit(1), bit(0)};
    std::vector<bit> or_op = {bit(0), bit(0), bit(0), bit(1)};
    std::vector<bit> and_op = {bit(1), bit(0), bit(0), bit(1)};

    // Mux ALU operations
    bit_vector_mux(decoded.alu_op, sub_op, decoded.is_sub | decoded.is_beq | decoded.is_bne);
    bit_vector_mux(decoded.alu_op, sll_op, decoded.is_sll | decoded.is_slli);
    bit_vector_mux(decoded.alu_op, slt_op, decoded.is_slt | decoded.is_slti | decoded.is_blt | decoded.is_bge);
    bit_vector_mux(decoded.alu_op, sltu_op, decoded.is_sltu | decoded.is_sltiu | decoded.is_bltu | decoded.is_bgeu);
    bit_vector_mux(decoded.alu_op, xor_op, decoded.is_xor | decoded.is_xori);
    bit_vector_mux(decoded.alu_op, srl_op, decoded.is_srl | decoded.is_srli);
    bit_vector_mux(decoded.alu_op, sra_op, decoded.is_sra | decoded.is_srai);
    bit_vector_mux(decoded.alu_op, or_op, decoded.is_or | decoded.is_ori);
    bit_vector_mux(decoded.alu_op, and_op, decoded.is_and | decoded.is_andi);

    // Set boolean flags using muxes
    decoded.is_alu_op = decoded.r_type.mux(bit(0), bit(1)).value();
    decoded.is_alu_op = decoded.i_alu.mux(bit(decoded.is_alu_op), bit(1)).value();
    decoded.is_alu_op = decoded.load.mux(bit(decoded.is_alu_op), bit(1)).value();
    decoded.is_alu_op = decoded.store.mux(bit(decoded.is_alu_op), bit(1)).value();
    decoded.is_alu_op = decoded.branch.mux(bit(decoded.is_alu_op), bit(1)).value();

    decoded.is_load = decoded.load.mux(bit(0), bit(1)).value();
    decoded.is_store = decoded.store.mux(bit(0), bit(1)).value();
    decoded.is_branch = decoded.branch.mux(bit(0), bit(1)).value();
    decoded.is_jump = decoded.jal.mux(bit(0), bit(1)).value();
    decoded.is_jump = decoded.jalr.mux(bit(decoded.is_jump), bit(1)).value();
    decoded.is_jalr = decoded.jalr.mux(bit(0), bit(1)).value();

    bit is_immediate = bit(0);
    is_immediate = is_immediate | decoded.i_alu;
    is_immediate = is_immediate | decoded.load;
    is_immediate = is_immediate | decoded.store;
    is_immediate = is_immediate | decoded.branch;
    is_immediate = is_immediate | decoded.jal;
    is_immediate = is_immediate | decoded.jalr;
    is_immediate = is_immediate | decoded.lui;
    is_immediate = is_immediate | decoded.auipc;
    decoded.is_immediate = is_immediate.value();

    decoded.is_i_imm = decoded.i_alu | decoded.load | decoded.jalr;
    decoded.is_s_imm = decoded.store;
    decoded.is_b_imm = decoded.branch;
    decoded.is_j_imm = decoded.jal;
    decoded.is_u_imm = decoded.lui | decoded.auipc;

    // Register fields
    decoded.rs1 = get_rs1(instruction);
    decoded.rs2 = get_rs2(instruction);
    decoded.rd = get_rd(instruction);

    // Create a 32-bit vector to store our immediate
    std::vector<bit> imm_bits(32, bit(0));

    // For each bit position
    // Optimize bit by storing it into an array and looping over it
    // Or at least try it out
    // for (int i = 0; i < 32; i++)
    // {
    //     bit curr_bit = bit(0);

    //     // Create individual bits for each immediate type
    //     bit i_imm_bit = bit((get_imm_i(instruction) >> i) & 1);
    //     bit s_imm_bit = bit((get_imm_s(instruction) >> i) & 1);
    //     bit b_imm_bit = bit((get_imm_b(instruction) >> i) & 1);
    //     bit j_imm_bit = bit((get_imm_j(instruction) >> i) & 1);

    //     // Mux each bit individually
    //     curr_bit = decoded.is_i_imm.mux(curr_bit, i_imm_bit);
    //     curr_bit = decoded.is_s_imm.mux(curr_bit, s_imm_bit);
    //     curr_bit = decoded.is_b_imm.mux(curr_bit, b_imm_bit);
    //     curr_bit = decoded.is_j_imm.mux(curr_bit, j_imm_bit);

    //     imm_bits[i] = curr_bit;
    // }

    // decoded.imm = 0;
    // for (int i = 0; i < 32; i++)
    // {
    //     decoded.imm |= (imm_bits[i].value() ? 1 : 0) << i;
    // }

    std::cout << "get_imm_u = " << get_imm_u(instruction) << std::endl;
    int32_t imm = 0;
    if (decoded.is_u_imm.value())
    {
        imm = get_imm_u(instruction);
    }
    else if (decoded.is_i_imm.value())
    {
        imm = get_imm_i(instruction);
    }
    else if (decoded.is_s_imm.value())
    {
        imm = get_imm_s(instruction);
    }
    else if (decoded.is_b_imm.value())
    {
        imm = get_imm_b(instruction);
    }
    else if (decoded.is_j_imm.value())
    {
        imm = get_imm_j(instruction);
    }
    decoded.imm = imm;

    return decoded;
}
