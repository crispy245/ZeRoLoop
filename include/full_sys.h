#pragma once

#include "../include/zero_loop.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>

extern bigint total_cost;

int32_t register_to_int(Register &reg);

void load_instructions(RAM *instr_mem, RAM *data_mem, const char *file_location, uint32_t data_start_addr = 2048);

void load_instructions(std::vector<uint32_t> &instr_mem, RAM *data_mem, const char *file_location, uint32_t data_start_addr = 2048);

void run_full_system(char *instr_location, bool ram_accurate = false, bool with_decoder = true);

