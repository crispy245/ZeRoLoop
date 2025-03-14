#pragma once

#include "c_headers.h"

class ZeroLoop
{
private:
    RegisterFile reg_file;
    Decoder decoder;
    ALU alu;
    PC pc;
    vector<uint32_t> *instruction_memory_fast;  // Pointer to instruction memory using uint32_t, faster
    RAM *instruction_memory_slow;               // Pointer to instruction memory using RAM, slower
    RAM *data_memory;                           // Pointer to data memory
    std::vector<Register> csrs;
    PLUGIN plugin;
    bigint start_count1;
    bigint end_count1;
    bigint start_count_only_cpu_1;
    bigint end_count_only_cpu_1;
    bigint total_cpu_gate_count;
    bigint total_cpu_gate_count_plus_mem;

public:
    // Constructor
    ZeroLoop(size_t num_registers = 32, size_t reg_width = 32)
        : reg_file(num_registers, reg_width),
          alu(),
          pc(0, reg_width),
          instruction_memory_fast(nullptr),
          instruction_memory_slow(nullptr),
          data_memory(nullptr),
          csrs(4096),
          start_count1(0),
          end_count1(0) ,
          start_count_only_cpu_1(0),
          end_count_only_cpu_1(0),
          total_cpu_gate_count(0),
          total_cpu_gate_count_plus_mem(0){}

    // Deep copy constructor
    ZeroLoop(const ZeroLoop &other)
        : reg_file(other.reg_file),
          decoder(other.decoder),
          alu(other.alu),
          pc(other.pc),
          instruction_memory_fast(other.instruction_memory_fast),
          instruction_memory_slow(other.instruction_memory_slow),
          data_memory(other.data_memory),
          csrs(other.csrs),
          start_count1(other.start_count1),
          end_count1(other.end_count1),
          start_count_only_cpu_1(other.start_count_only_cpu_1),
          end_count_only_cpu_1(other.end_count_only_cpu_1),
          total_cpu_gate_count(other.total_cpu_gate_count),
          total_cpu_gate_count_plus_mem(other.total_cpu_gate_count_plus_mem){}

    void copy_state_from(const ZeroLoop &other)
    {
        reg_file = other.reg_file;
        pc = other.pc;
        instruction_memory_fast = other.instruction_memory_fast;
        instruction_memory_slow = other.instruction_memory_slow;
        data_memory = other.data_memory;
        start_count1 = other.start_count1;
        end_count1   = other.end_count1;
        start_count_only_cpu_1 = other.start_count_only_cpu_1;
        end_count_only_cpu_1 = other.end_count_only_cpu_1;
        total_cpu_gate_count = other.total_cpu_gate_count;
        total_cpu_gate_count_plus_mem = other.total_cpu_gate_count_plus_mem;
    }

    // RegisterFile operations
    Register read_register(size_t pos);
    void write_register(size_t pos, Register a);
    Register at_register(size_t index);
    size_t get_register_width();
    void print_registers();

    // ALU operations
    Register execute_alu(Register &a, Register &b, std::vector<bit> alu_op);
    Register execute_alu_partial(Register &a, Register &b, std::vector<bit> alu_op);
    void subtract(Register &result, Register a, Register b);

    // PLUGIN operations
    Register execute_plug_in_unit(Register &ret, Register a, Register b,    uint32_t funct3, uint32_t funct7, uint32_t opcode);

    // Conditional write to units
    void conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, std::vector<bit> f3_bits);
    void conditional_memory_write(const bit &should_write, const std::vector<bit> &addr, const std::vector<bit> &data, uint32_t f3_bits);
    Register conditional_memory_read(const bit &should_read, const std::vector<bit> &addr, std::vector<bit> f3_bits);
    Register conditional_memory_read(const bit &should_read, const std::vector<bit> &addr, uint32_t f3_bits);


    void conditional_register_write(const bit &should_write, size_t rd, const Register &data);
    void conditional_register_write(const bool should_write, size_t rd, const Register &data);
    void conditional_csr_write(const bit &should_write, size_t csr_pos, const Register &data);
 
    void full_adder(bit &s, bit &c, bit a, bit b, bit cin);
    void add(Register &ret, Register a, Register b);
    uint32_t get_pc() { return pc.read_pc(); };

    // Stage operations
    void execute_instruction_with_decoder(uint32_t instruction);
    void execute_instruction_with_decoder_optimized(uint32_t instruction);

    void execute_instruction_without_decoder(uint32_t instruction);
    void connect_memories(vector<uint32_t> *instr_mem, RAM *data_mem);
    void connect_memories(RAM *instr_mem, RAM *data_mem);
    void run_program();

    // syscalls
    void handle_syscall();

    // Measure gate count
    void check_for_counter(uint32_t instr, bool start_or_end);

    // print info
    void print_details();
};
