#pragma once

#include <cassert>
#include "ram.h"
#include "bit_vector.h"
#include "register.h"

class RAM 
{
private:
    vector<vector<bit>> memory;
    size_t word_size;
    size_t addr_bits;

public:
    RAM(size_t size = 1024, size_t word_size = 32) : word_size(word_size) {
        // Initialize memory with 'size' words of 'word_size' bits each
        memory = vector<vector<bit>>(size, vector<bit>(word_size, bit(0)));
        addr_bits = 0;
        while ((1u << addr_bits) < size) addr_bits++;
    }

    vector<bit> read(const vector<bit>& address) {
        return ram_read(memory, address);
    }

    void write(const vector<bit>& address, const vector<bit>& data) {
        ram_write(memory, address, data);
    }

    size_t get_word_size() { return word_size; }
    size_t get_addr_bits() { return addr_bits; }
};