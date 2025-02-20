// c_headers.h
#pragma once


#define INSTR_MEM_SIZE 131072
#define DATA_MEM_SIZE 131072 // grows upwards

#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

#include "register.h"
#include "reg_file.h"
#include "alu.h"
#include "pc.h"
#include "ram_cpu.h"
#include "decoder.h"
#include "plugin.h"