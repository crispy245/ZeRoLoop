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

# Python configuration
PYTHON = python3
VMH_SCRIPT = objdump2vmh.py

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(INC_DIR)/*.h)
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

# Linker flags
LDFLAGS = -T memory_map.ld -lgcc

# Create directory structure
$(shell mkdir -p $(SRC_DIR) $(INC_DIR) $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR) $(DUMP_DIR) $(VMH_DIR))

.PHONY: all clean depend

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

clean:
	rm -f $(OBJS) $(DEPS)
	rm -rf $(OBJ_DIR)/* $(DEP_DIR)/* $(BIN_DIR)/* $(DUMP_DIR)/* $(VMH_DIR)/*

# Header file dependency checking
depend: $(DEPS)

# Include generated dependencies
-include $(DEPS)