#include "register.h"
#include "bit_vector.h" // For bit_vector_from_integer function

Register::Register(size_t width) : data(width, bit(0))
{
    // Initialize register with specified width, all bits set to 0
}

Register::Register(std::vector<bit> &bits) : data(bits)
{
    // Initialize register with existing bit vector
}

Register::Register(bigint value, size_t width) : data(width, bit(0))
{
    // Convert integer value to bits and store in register
    std::vector<bit> value_bits = bit_vector_from_integer(value);

    // Copy bits, ensuring we don't exceed register width
    for (bigint i = 0; i < width && i < value_bits.size(); i++)
    {
        data[i] = value_bits[i];
    }
}

Register::Register(int32_t value, size_t width) : data(width, bit(0))
{
    std::vector<bit> value_bits = bit_vector_from_int32(value);
    for (size_t i = 0; i < width && i < value_bits.size(); i++)
    {
        data[i] = value_bits[i];
    }
    // Sign extend if width is greater than 32
    if (width > 32 && (value < 0))
    {
        for (size_t i = 32; i < width; i++)
        {
            data[i] = bit(1);
        }
    }
}

Register::Register(uint32_t value, size_t width) : data(width, bit(0)) {
    std::vector<bit> value_bits = bit_vector_from_int32(value);
    for (size_t i = 0; i < width && i < value_bits.size(); i++) {
        data[i] = value_bits[i];
    }
}

bit Register::get_bit(size_t index) const
{
    return data[index];
}

size_t Register::width() const
{
    return data.size();
}

void Register::print(const std::string &name) const
{
    std::cout << name << ": ";
    for (int i = width() - 1; i >= 0; i--)
    {
        std::cout << data[i].value();
    }
    std::cout << std::endl;
}

bit &Register::at(size_t index)
{
    return data.at(index);
}

const bit &Register::at(size_t index) const
{
    return data.at(index);
}

void Register::push_back(const bit &b)
{
    data.push_back(b);
}

const std::vector<bit> &Register::get_data()
{
    return data;
}

uint32_t Register::get_data_uint() const
{
    assert(data.size() <= 32 && "Register size must be 32 bits or less");

    uint32_t result = 0;
    for (size_t i = 0; i < data.size(); i++)
    {
        if (data[i].value())
        {
            result |= (1u << i);
        }
    }
    return result;
}

void Register::update_data(bigint new_data)
{
    Register new_data_reg(new_data, data.size());
    for (bigint i = 0; i < data.size(); i++)
    {
        data.at(i) = new_data_reg.at(i);
    }
}