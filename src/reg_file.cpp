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
    
    for (size_t i = 0; i < register_list.size(); i++) {
        cout << "R" << left << setw(2) << i << ": ";
        
        for (int j = register_list[i].width() - 1; j >= 0; j--) {
            cout << register_list[i].get_bit(j).value();
        }
        cout << '\n';
    }
    cout << endl;
}