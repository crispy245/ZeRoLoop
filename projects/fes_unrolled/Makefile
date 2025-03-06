#===============================================================================
# Emulator Build Section (C++ emulator)
#===============================================================================


EMU_SRCDIR    := ../../src
EMU_INCDIR    := ../../include
EMU_BUILDDIR  := build

EMU_CXX       := g++
EMU_CXXFLAGS  := -O3 -I$(EMU_INCDIR) -std=c++17 -g
EMU_LDFLAGS   := -lgmp

# Get all emulator .cpp files from EMU_SRCDIR except main.cpp and plugin.cpp.
EMU_SRCS      := $(filter-out $(EMU_SRCDIR)/main.cpp $(EMU_SRCDIR)/plugin.cpp, $(wildcard $(EMU_SRCDIR)/*.cpp))
EMU_OBJS      := $(patsubst %.cpp,$(EMU_BUILDDIR)/%.o,$(notdir $(EMU_SRCS)))
EMU_PLUGIN_OBJ:= $(EMU_BUILDDIR)/plugin.o
EMU_MAIN_OBJ  := $(EMU_BUILDDIR)/main.o

EMU_ALL_OBJS  := $(EMU_OBJS) $(EMU_PLUGIN_OBJ)

DECODE 		  := true

# Final emulator executable (in build folder)
$(EMU_BUILDDIR)/program: $(EMU_MAIN_OBJ) $(EMU_ALL_OBJS) | $(EMU_BUILDDIR)
	$(EMU_CXX) $(EMU_CXXFLAGS) $^ -o $@ $(EMU_LDFLAGS)

# Ensure build directory exists
$(EMU_BUILDDIR):
	mkdir -p $(EMU_BUILDDIR)

# Compile main.cpp from EMU_SRCDIR
$(EMU_BUILDDIR)/main.o: $(EMU_SRCDIR)/main.cpp | $(EMU_BUILDDIR)
	$(EMU_CXX) $(EMU_CXXFLAGS) -c $< -o $@

# Pattern rule to compile emulator source files into build objects
$(EMU_BUILDDIR)/%.o: $(EMU_SRCDIR)/%.cpp | $(EMU_BUILDDIR)
	$(EMU_CXX) $(EMU_CXXFLAGS) -c $< -o $@

# Compile plugin.cpp (from current folder)
$(EMU_BUILDDIR)/plugin.o: plugin.cpp | $(EMU_BUILDDIR)
	$(EMU_CXX) $(EMU_CXXFLAGS) -c $< -o $@

# Debug, run, and accurate targets for the emulator
debug: CXXFLAGS := -O0 -I$(EMU_INCDIR) -std=c++17 -g -fno-inline-small-functions
debug: $(EMU_BUILDDIR)/program
	gdb $(EMU_BUILDDIR)/program

run: $(EMU_BUILDDIR)/program
	./$(EMU_BUILDDIR)/program vmh/main.rv32.elf.vmh false $(DECODE)

accurate: $(EMU_BUILDDIR)/program
	./$(EMU_BUILDDIR)/program vmh/main.rv32.elf.vmh true $(DECODE)


#===============================================================================
# RISC‑V Code Generation Section (C code)
#===============================================================================


# Toolchain configuration
RV_PREFIX     := riscv32-unknown-elf-
RV_CC         := $(RV_PREFIX)gcc
RV_AS         := $(RV_PREFIX)as
RV_LD         := $(RV_PREFIX)ld
RV_OBJCOPY    := $(RV_PREFIX)objcopy
RV_OBJDUMP    := $(RV_PREFIX)objdump
RV_SIZE       := $(RV_PREFIX)size

# Project configuration
RV_PROJECT    := main
RV_TARGET     := $(RV_PROJECT).rv32.elf
RV_BIN        := $(RV_PROJECT).bin
RV_VMH        := $(RV_PROJECT).vmh
RV_DUMP       := $(RV_PROJECT).dump

# Directory structure for RISC‑V build
RV_SRC_DIR    := src
RV_INC_DIR    := include
RV_OBJ_DIR    := obj
RV_DEP_DIR    := dep
RV_BIN_DIR    := bin
RV_DUMP_DIR   := dump
RV_VMH_DIR    := vmh
RV_ASM_DIR    := asm
RV_CODE_GEN   := gen_unrolled.py  # Script for code generation

# Code generation parameters (defaults)
N             ?= 5  # Default value for n parameter
M             ?= 10  # Default value for m parameter
PRINT_FLAG    ?= 0  # Default value for print flag (0=False, 1=True)

# Python configuration
RV_PYTHON     := python3
RV_VMH_SCRIPT := ../../c/objdump2vmh.py 
# Source files
RV_SRCS       := $(wildcard $(RV_SRC_DIR)/*.c)
RV_ASM_SRCS   := ../../c/asm/crt0.S  # Startup code
RV_HEADERS    := $(wildcard $(RV_INC_DIR)/*.h)
RV_OBJS       := $(RV_ASM_SRCS:$(RV_ASM_DIR)/%.s=$(RV_OBJ_DIR)/%.o) \
                 $(RV_SRCS:$(RV_SRC_DIR)/%.c=$(RV_OBJ_DIR)/%.o)
RV_DEPS       := $(RV_SRCS:$(RV_SRC_DIR)/%.c=$(RV_DEP_DIR)/%.d)

# Compiler and assembler flags
RV_CFLAGS   := -march=rv32i_zicsr \
               -mabi=ilp32 \
               -nostartfiles \
               -fno-exceptions \
               -fno-inline \
               -Wall       \
               -Wextra     \
               -O3         \
               -g \
               -I$(RV_INC_DIR)
RV_ASFLAGS  := -march=rv32i_zicsr -I$(RV_INC_DIR)

# RISC‑V test flags (if needed)
RV_TEST_FLAGS := -march=rv32izicsr \
                 -mabi=ilp32 \
                 -nostdlib \
                 -nostartfiles \
                 -I$(RV_INC_DIR) \
                 -I$(RISCV_TESTS_DIR)

# Linker flags for RISC‑V build (renamed to avoid conflict)
RV_LDFLAGS  := -T ../../c/memory_map.ld -lgcc

# Create RISC‑V directories (if they don't exist)
$(shell mkdir -p $(RV_SRC_DIR) $(RV_INC_DIR) $(RV_OBJ_DIR) $(RV_DEP_DIR) $(RV_BIN_DIR) $(RV_DUMP_DIR) $(RV_VMH_DIR) $(RISCV_TESTS_VMH))

.PHONY: code depend test asm generate-code all

# New target to first generate code
all: generate-code code

# Generate code and save to src/main.c
generate-code:
	@echo "Generating code with n=$(N), m=$(M), print=$(PRINT_FLAG)..."
	@if [ -f $(RV_CODE_GEN) ]; then \
		if [ "$(PRINT_FLAG)" = "1" ]; then \
			$(RV_PYTHON) $(RV_CODE_GEN) $(N) $(M) --print > $(RV_SRC_DIR)/main.c; \
		else \
			$(RV_PYTHON) $(RV_CODE_GEN) $(N) $(M) > $(RV_SRC_DIR)/main.c; \
		fi; \
		echo "Code generated and saved to $(RV_SRC_DIR)/main.c"; \
	else \
		echo "Warning: Code generation script $(RV_CODE_GEN) not found."; \
		echo "Please create this file or manually place your code in $(RV_SRC_DIR)/main.c"; \
	fi

# Main target for RISC‑V code generation
code: $(RV_BIN_DIR)/$(RV_TARGET) generate-dumps move-outputs

# Compile assembly files
$(RV_OBJ_DIR)/%.o: $(RV_ASM_DIR)/%.s
	$(RV_CC) $(RV_CFLAGS) -c $< -o $@

# Compile C files
$(RV_OBJ_DIR)/%.o: $(RV_SRC_DIR)/%.c
	$(RV_CC) $(RV_CFLAGS) -c $< -o $@
	@$(RV_CC) $(RV_CFLAGS) -MM -MT $@ $< > $(RV_DEP_DIR)/$*.d

# Link the ELF file (ensure startup code comes first)
$(RV_BIN_DIR)/$(RV_TARGET): $(RV_OBJS)
	$(RV_CC) $(RV_CFLAGS) $(RV_LDFLAGS) $(RV_OBJS) -o $@

# Generate dumps and VMH file from the ELF
generate-dumps: $(RV_BIN_DIR)/$(RV_TARGET)
	$(RV_OBJDUMP) -EL -sz --section=.text --section=.data --section=.rodata $< > $(RV_BIN_DIR)/$(RV_TARGET).dump
	$(RV_OBJDUMP) -D -S -EL --source --section=.text --section=.data --section=.rodata $< > $(RV_BIN_DIR)/$(RV_TARGET).detailed.dump
	$(RV_PYTHON) $(RV_VMH_SCRIPT) $(RV_BIN_DIR)/$(RV_TARGET).dump > $(RV_BIN_DIR)/$(RV_TARGET).vmh

# Move generated dump and VMH files to their directories
move-outputs:
	mv $(RV_BIN_DIR)/$(RV_TARGET).dump $(RV_DUMP_DIR)/
	mv $(RV_BIN_DIR)/$(RV_TARGET).detailed.dump $(RV_DUMP_DIR)/
	mv $(RV_BIN_DIR)/$(RV_TARGET).vmh $(RV_VMH_DIR)/

# Pattern rule for assembling .S files (for test purposes)
$(RV_BIN_DIR)/%.elf: $(RV_ASM_DIR)/%.S
	@echo "Building $*..."
	$(RV_CC) $(RV_TEST_FLAGS) -T ../../c/memory_map.ld -nostdlib $< -o $@
	$(RV_OBJDUMP) -EL -sz --section=.text --section=.data --section=.rodata $@ > $(RV_DUMP_DIR)/$*.dump
	$(RV_OBJDUMP) -D -S -EL --source --section=.text --section=.data --section=.rodata $@ > $(RV_DUMP_DIR)/$*.detailed.dump
	$(RV_PYTHON) $(RV_VMH_SCRIPT) $(RV_DUMP_DIR)/$*.dump > $(RV_VMH_DIR)/$(RV_TARGET).vmh

# Target to build a .elf from an assembly file (e.g. main.S)
asm: $(RV_BIN_DIR)/main.elf
	@echo "Compiled main.S, generated dumps, and moved outputs."

# Dependency tracking for C files
depend: $(RV_DEPS)
-include $(RV_DEPS)

#===============================================================================
# Clean target for both builds
#===============================================================================
clean:
	@echo "Cleaning emulator and RISC-V build artifacts..."
	# Clean emulator build files
	rm -rf $(EMU_BUILDDIR)
	# Clean RISC-V build directories
	rm -rf $(RV_OBJ_DIR) $(RV_DEP_DIR) $(RV_BIN_DIR) $(RV_DUMP_DIR) $(RV_VMH_DIR) $(RISCV_TESTS_VMH)

.PHONY: clean run debug