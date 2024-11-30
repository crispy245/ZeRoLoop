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
VMH_SCRIPT = objdump2vmh.py

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(INC_DIR)/*.h)
ASM_TESTS = $(wildcard $(RISCV_TESTS_DIR)/*.S)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# Compiler flags
CFLAGS = -march=rv32izicsr \
         -mabi=ilp32 \
         -nostdlib \
         -ffreestanding \
         -nostartfiles \
         -fno-exceptions \
         -Wall \
         -Wextra \
         -O2 \
         -g \
         -I$(INC_DIR)

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

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MT $@ $< > $(DEP_DIR)/$*.d

# Link the ELF file
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

# Generate dumps and VMH files
generate-dumps: $(BIN_DIR)/$(TARGET)
	$(OBJDUMP) -EL -sz --section=.xcpthandler --section=.text --section=.data $< > $(BIN_DIR)/$(TARGET).dump
	$(PYTHON) $(VMH_SCRIPT) $(BIN_DIR)/$(TARGET).dump > $(BIN_DIR)/$(TARGET).vmh

# Move outputs to appropriate directories
move-outputs:
	mv $(BIN_DIR)/*.dump $(DUMP_DIR)/
	mv $(BIN_DIR)/*.vmh $(VMH_DIR)/

# RISC-V test compilation target
test: $(ASM_TESTS)
	@echo "Building RISC-V tests..."
	@for test in $(basename $(notdir $(ASM_TESTS))); do \
		echo "Building $$test..."; \
		$(CC) $(TEST_FLAGS) -T memory_map.ld -nostdlib \
			$(RISCV_TESTS_DIR)/$$test.S -o $(BIN_DIR)/$$test.elf && \
		$(OBJDUMP)  -EL -sz --section=.xcpthandler --section=.text --section=.data $(BIN_DIR)/$$test.elf > $(DUMP_DIR)/$$test.dump && \
		$(PYTHON) $(VMH_SCRIPT) $(DUMP_DIR)/$$test.dump > $(RISCV_TESTS_VMH)/$$test.vmh; \
	done

clean:
	rm -f $(OBJS) $(DEPS)
	rm -rf $(OBJ_DIR)/* $(DEP_DIR)/* $(BIN_DIR)/* $(DUMP_DIR)/* $(VMH_DIR)/* $(RISCV_TESTS_VMH)/*

# Header file dependency checking
depend: $(DEPS)

# Include generated dependencies
-include $(DEPS)