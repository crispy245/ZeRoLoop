#pragma once

#include "bit.h"
#include <vector>
#include <string>
#include <iostream>

class Register
{
private:
    std::vector<bit> data;

    static std::vector<bit> bit_vector_from_int32(int32_t value) // should rename
    {
        std::vector<bit> result(32);
        // Handle both positive and negative numbers
        for (int i = 0; i < 32; i++)
        {
            result[i] = bit((value >> i) & 1);
        }
        return result;
    }

public:
    explicit Register(size_t width = 32);
    Register(std::vector<bit> &bits);
    Register(bigint value, size_t width);
    Register(uint32_t value, size_t width);
    Register(int32_t value, size_t width);

    bit get_bit(size_t index) const;
    size_t width() const;
    void print(const std::string &name) const;
    bit &at(size_t index);
    const bit &at(size_t index) const;
    void push_back(const bit &b);
    const std::vector<bit> &get_data();
    void update_data(bigint new_data);
    uint32_t get_data_uint() const;

     // Overload () operator to return a vector of bits
    std::vector<bit> operator()(size_t start, size_t end) const;
    
    // Overload () operator to return a uint32_t
    uint32_t operator()(size_t start, size_t end, bool as_uint32_t) const;
};
