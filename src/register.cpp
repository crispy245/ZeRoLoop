#include "register.h"
#include "bit_vector.h" // For bit_vector_from_integer function

Register::Register(size_t width) : data(width, bit(0)) {
    // Initialize register with specified width, all bits set to 0
}

Register::Register(std::vector<bit>& bits) : data(bits) {
    // Initialize register with existing bit vector
}

Register::Register(bigint value, size_t width) : data(width, bit(0)) {
    // Convert integer value to bits and store in register
    std::vector<bit> value_bits = bit_vector_from_integer(value);
    
    // Copy bits, ensuring we don't exceed register width
    for (bigint i = 0; i < width && i < value_bits.size(); i++) {
        data[i] = value_bits[i];
    }
}

bit Register::get_bit(size_t index) const {
    return data[index];
}

size_t Register::width() const {
    return data.size();
}

void Register::print(const std::string& name) const {
    std::cout << name << ": ";
    for (int i = width() - 1; i >= 0; i--) {
        std::cout << data[i].value();
    }
    std::cout << std::endl;
}

bit& Register::at(size_t index) {
    return data.at(index);
}

const bit& Register::at(size_t index) const {
    return data.at(index);
}

void Register::push_back(const bit& b) {
    data.push_back(b);
}

const std::vector<bit>& Register::get_data(){
    return data;
}