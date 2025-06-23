#include "reg_file.h"
#include <iomanip>

Register RegisterFile::at(size_t index)
{
    assert(index < register_list.size());
    return register_list.at(index);
}

size_t RegisterFile::register_width()
{
    return register_list.size();
}

void RegisterFile::write(size_t pos, Register a)
{
    register_list[pos] = a;
}

Register RegisterFile::read(size_t pos)
{
    return register_list[pos];
}

void RegisterFile::print_all_contents() {
    cout << "\nRegister File Contents:\n";
    cout << "----------------------\n";
    
    // RISC-V register names
    const string register_names[32] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };
    
    for (size_t i = 0; i < register_list.size() && i < 32; i++) {
        cout << left << setw(4) << register_names[i] << ": ";
        
        for (int j = register_list[i].width() - 1; j >= 0; j--) {
            cout << register_list[i].get_bit(j).value();
        }
        cout << '\n';
    }
    cout << endl;
}