# ZeRoLoop, the Non-Cyclic RISC-V Emulator

## Prerequisites

- **RISC-V Toolchain**: Install `riscv32-unknown-elf-` tools (GCC, assembler, linker, etc.).
- **GCC Compiler**
- **Python 3**: Required for VMH generation scripts.

## Building and Running

In order to run the emulator you must generate binaries and then load them into the processor.

```bash 
cd c
make
cd ../
make run
```

We provided a small program in C named `main.c`. Please modify with the code you wish to benchmark.

## Running compliance suite

We included a compliance suit, to compile and execute please run the following commands

```bash
cd c
make test
cd ../
make test-all
```


