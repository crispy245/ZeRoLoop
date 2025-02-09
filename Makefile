CXX = g++
CXXFLAGS = -O3 -I./include -std=c++17 -g
LDFLAGS = -lgmp
SOURCES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)
MAIN_OBJ = src/main.o
TEST_LOG = test_results.txt
DECODE = true


program: $(MAIN_OBJ) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS := -O0 -I./include -std=c++17 -g
debug: program
	gdb ./program

run: program
	./program c/vmh/main.rv32.elf.vmh false $(DECODE)

accurate: program
	./program c/vmh/main.rv32.elf.vmh true $(DECODE)


test-all: program
	@echo "Running all VMH tests from riscv_tests_vmh directory..." | tee $(TEST_LOG)
	@echo "Test started at $$(date)" >> $(TEST_LOG)
	@START=$$(date +%s%N); \
	for file in c/riscv_tests_vmh/*.vmh; do \
		echo "\nTesting $${file}..." | tee -a $(TEST_LOG); \
		output=$$(./program "$${file}" 2>&1 false); \
		exit_code=$$(echo "$$output" | grep -oP 'Program exited with code \K\-?[0-9]+'); \
		if [ -z "$$exit_code" ]; then \
			echo "✗ FAIL: $${file} (No exit code found in output)" | tee -a $(TEST_LOG); \
		elif [ $$exit_code -eq 0 ]; then \
			echo "✓ PASS: $${file}" | tee -a $(TEST_LOG); \
		else \
			echo "✗ FAIL: $${file} (Exit code: $$exit_code)" | tee -a $(TEST_LOG); \
		fi; \
	done; \
	END=$$(date +%s%N); \
	DURATION_NS=$$((END - START)); \
	DURATION=$$(awk "BEGIN { printf \"%.3f\", $$DURATION_NS / 1000000000 }"); \
	printf "\nAll tests completed at $$(date)\nTotal time taken: \033[1m%s seconds\033[0m\n" "$$DURATION" | tee -a $(TEST_LOG)

clean:
	rm -f program $(TEST_LOG) $(OBJECTS) $(MAIN_OBJ)

.PHONY: clean run test-all debug