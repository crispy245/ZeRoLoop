// c_headers.h
#pragma once


#define INSTR_MEM_SIZE 1048576
#define DATA_MEM_BASE (INSTR_MEM_SIZE*4) // grows upwards

#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <bigint.h>

#include "register.h"
#include "reg_file.h"
#include "alu.h"
#include "pc.h"
#include "ram_cpu.h"
#include "decoder.h"
#include "plugin.h"