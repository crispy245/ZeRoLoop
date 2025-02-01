# Toolchain configuration
PREFIX = riscv32-unknown-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# Project configuration
PROJECT = main
TARGET = $(PROJECT).rv32.elf
BIN = $(PROJECT).bin
VMH = $(PROJECT).vmh
DUMP = $(PROJECT).dump

# Directory structure
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
DEP_DIR = dep
BIN_DIR = bin
DUMP_DIR = dump
VMH_DIR = vmh
ASM_DIR = asm
RISCV_TESTS_DIR = riscv_tests
RISCV_TESTS_VMH = riscv_tests_vmh

# Python configuration
PYTHON = python3
VMH_SCRIPT = objdump2vmh.py  # Ensure this script exists in your project directory

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
ASM_SRCS = $(ASM_DIR)/crt0.s  # Add startup code
HEADERS = $(wildcard $(INC_DIR)/*.h)
ASM_TESTS = $(wildcard $(RISCV_TESTS_DIR)/*.S)
OBJS = $(ASM_SRCS:$(ASM_DIR)/%.s=$(OBJ_DIR)/%.o) \
       $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# Compiler flags
CFLAGS = -march=rv32i_zicsr \
    -mabi=ilp32 \
    -nostdlib \
    -ffreestanding \
    -nostartfiles \
    -fno-exceptions \
    -fno-inline \
    -Wall \
    -Wextra \
    -O0 \
    -g \
    -I$(INC_DIR)

# Assembler flags
ASFLAGS = -march=rv32i_zicsr -I$(INC_DIR)

# RISC-V test flags
TEST_FLAGS = -march=rv32izicsr \
    -mabi=ilp32 \
    -nostdlib \
    -nostartfiles \
    -I$(INC_DIR) \
    -I$(RISCV_TESTS_DIR)

# Linker flags
LDFLAGS = -T memory_map.ld -lgcc

# Create directory structure
$(shell mkdir -p $(SRC_DIR) $(INC_DIR) $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR) $(DUMP_DIR) $(VMH_DIR) $(RISCV_TESTS_VMH))

.PHONY: all clean depend test

all: $(BIN_DIR)/$(TARGET) generate-dumps move-outputs

# Compile assembly files (ensure crt0.s is compiled first)
$(OBJ_DIR)/%.o: $(ASM_DIR)/%.s
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MT $@ $< > $(DEP_DIR)/$*.d

# Link the ELF file (ensure crt0.o comes first)
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

# Generate dumps and VMH files
generate-dumps: $(BIN_DIR)/$(TARGET)
	$(OBJDUMP) -EL -sz --section=.text --section=.data --section=.rodata $< > $(BIN_DIR)/$(TARGET).dump
	$(OBJDUMP) -D -S -EL --source --section=.text --section=.data --section=.rodata $< > $(BIN_DIR)/$(TARGET).detailed.dump
	$(PYTHON) $(VMH_SCRIPT) $(BIN_DIR)/$(TARGET).dump > $(BIN_DIR)/$(TARGET).vmh

# Move outputs to appropriate directories
move-outputs:
	mv $(BIN_DIR)/*.dump $(DUMP_DIR)/
	mv $(BIN_DIR)/*.vmh $(VMH_DIR)/

# Extract test names (without directory and extension)
TEST_NAMES := $(basename $(notdir $(ASM_TESTS)))

# Pattern rule: Build each test's ELF file and generate associated dumps and VMH.
$(BIN_DIR)/%.elf: $(RISCV_TESTS_DIR)/%.S
	@echo "Building $*..."
	$(CC) $(TEST_FLAGS) -T memory_map.ld -nostdlib $< -o $@
	$(OBJDUMP) -EL -sz --section=.text --section=.data --section=.rodata $@ > $(DUMP_DIR)/$*.dump
	$(OBJDUMP) -D -S -EL --source --section=.text --section=.data --section=.rodata $@ > $(DUMP_DIR)/$*.detailed.dump
	$(PYTHON) $(VMH_SCRIPT) $(DUMP_DIR)/$*.dump > $(RISCV_TESTS_VMH)/$*.vmh

# The 'test' target depends on all individual test ELF files.
test: $(TEST_NAMES:%=$(BIN_DIR)/%.elf)
	@echo "All RISC-V tests have been built."

clean:
	rm -f $(OBJS) $(DEPS)
	rm -rf $(OBJ_DIR)/* $(DEP_DIR)/* $(BIN_DIR)/* $(DUMP_DIR)/* $(VMH_DIR)/* $(RISCV_TESTS_VMH)/*

# Header file dependency checking
depend: $(DEPS)

# Include generated dependencies
-include $(DEPS)