CXX = g++
CXXFLAGS = -O3 -I./include -std=c++17 -g
LDFLAGS = -lgmp
SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
TEST_LOG = test_results.txt

program: src/main.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

debug: CXXFLAGS += -O0
debug: program
	gdb ./program

run: clean program
	./program c/vmh/main.rv32.elf.vmh

test-all: clean program
	@echo "Running all VMH tests from riscv_tests_vmh directory..." | tee $(TEST_LOG)
	@echo "Test started at $$(date)" >> $(TEST_LOG)
	@for file in c/riscv_tests_vmh/*.vmh; do \
		echo "\nTesting $${file}..." | tee -a $(TEST_LOG); \
		if ./program "$${file}"; then \
			echo "✓ PASS: $${file}" >> $(TEST_LOG); \
		else \
			echo "✗ FAIL: $${file}" >> $(TEST_LOG); \
			exit 1; \
		fi; \
	done
	@echo "\nAll tests completed at $$(date)" | tee -a $(TEST_LOG)
	@echo "Test results saved to $(TEST_LOG)"

clean:
	rm -f program $(TEST_LOG)