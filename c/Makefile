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
DEP_DIR = dep
BIN_DIR = bin
DUMP_DIR = dump
VMH_DIR = vmh

# Python configuration
PYTHON = python3
VMH_SCRIPT = objdump2vmh.py

# Source files
SRCS = main.c
OBJS = $(SRCS:.c=.o)

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
         -g

# Linker flags
LDFLAGS = -T memory_map.ld

# Create directory structure
$(shell mkdir -p $(DEP_DIR) $(BIN_DIR) $(DUMP_DIR) $(VMH_DIR))

.PHONY: all clean depend

all: $(BIN_DIR)/$(TARGET) generate-dumps move-outputs

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -MM $< > $(DEP_DIR)/$*.d

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
	-mv *.d $(DEP_DIR)/ 2>/dev/null || true

clean:
	rm -f $(OBJS)
	rm -f *.o *.bin *.hex *.lst *.elf *.map main
	rm -rf $(DEP_DIR) $(BIN_DIR) $(DUMP_DIR) $(VMH_DIR)

# Include dependency files
-include $(SRCS:%.c=$(DEP_DIR)/%.d)