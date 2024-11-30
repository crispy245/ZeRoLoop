CXX = g++
CXXFLAGS = -I./include -std=c++17 -g
LDFLAGS = -lgmp
SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))

program: src/main.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

debug: CXXFLAGS += -O0
debug: program
	gdb ./program

run: clean program
	./program c/vmh/main.rv32.elf.vmh

test-all: clean program
	@echo "Running all VMH tests from riscv_tests_vmh directory..."
	@for file in c/riscv_tests_vmh/*.vmh; do \
		echo "\nTesting $${file}..."; \
		./program "$${file}" || exit 1; \
	done
	@echo "\nAll tests completed."

clean:
	rm -f program