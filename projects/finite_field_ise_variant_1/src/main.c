#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"

int main()
{
    volatile int mod = 128;
    // Predefined test pairs (a, b)
    volatile int test_pairs[][2] = {
        {0, 0}, {5, 3}, {127, 1}, {128, 128}, {129, 50},
        {255, 127}, {256, 200}, {257, 15}, {500, 300}, {1000, 900},
        {1337, 42}, {2048, 1024}, {4096, 128}, {65536, 256}, {431, 421},
        {542, 52}, {4123, 5421}, {531, 41236}, {6345, 526}, {764, 745},
        {432, 1}, {7, 8}, {100, 27}, {50, 75}, {300, 400},
        {1, 127}, {63, 64}, {200, 100}, {999, 1}, {128, 1}
    };
    
    int num_test_pairs = sizeof(test_pairs) / sizeof(test_pairs[0]);
    
    print_str("=======================================================\n");
    print_str("================ TESTING ADD MOD =====================\n");
    print_str("=======================================================\n");
    
    // Test ADD MOD (funct3=3, funct7=11)
    for (int i = 0; i < num_test_pairs; i++) {
        volatile int a = test_pairs[i][0];
        volatile int b = test_pairs[i][1];
        volatile int sw_result, hw_result;
        
        // Software: add first, then reduce
        sw_result = (a + b) % mod;
        
        // Hardware: add and reduce in one call
        hw_result = cfu_op_hw(3, 11, a, b);
        
        print_str("ADD MOD: a=");
        print_int(a);
        print_str(", b=");
        print_int(b);
        print_str(", sw=");
        print_int(sw_result);
        print_str(", hw=");
        print_int(hw_result);
        print_str("\n");
        
        if (sw_result != hw_result) {
            print_str("*** FAILED ***\n");
            return 0;
        }
    }
    
    print_str("=======================================================\n");
    print_str("================ TESTING SUB MOD =====================\n");
    print_str("=======================================================\n");
    
    // Test SUB MOD (funct3=1, funct7=11)
    for (int i = 0; i < num_test_pairs; i++) {
        volatile int a = test_pairs[i][0];
        volatile int b = test_pairs[i][1];
        volatile int sw_result, hw_result;
        
        // Software: subtract first, then reduce (handle negative results)
        int temp = a - b;
        sw_result = ((temp % mod) + mod) % mod;  // Handle negative modulo
        
        // Hardware: subtract and reduce in one call
        hw_result = cfu_op_hw(1, 11, a, b);
        
        print_str("SUB MOD: a=");
        print_int(a);
        print_str(", b=");
        print_int(b);
        print_str(", sw=");
        print_int(sw_result);
        print_str(", hw=");
        print_int(hw_result);
        print_str("\n");
        
        if (sw_result != hw_result) {
            print_str("*** FAILED ***\n");
            return 0;
        }
    }
    
    print_str("=======================================================\n");
    print_str("================ TESTING MULT MOD ====================\n");
    print_str("=======================================================\n");
    
    // Test MULT MOD (funct3=2, funct7=11)
    for (int i = 0; i < num_test_pairs; i++) {
        volatile int a = test_pairs[i][0];
        volatile int b = test_pairs[i][1];
        volatile int sw_result, hw_result;
        
        // Software: multiply first, then reduce
        sw_result = (a * b) % mod;
        
        // Hardware: multiply and reduce in one call
        hw_result = cfu_op_hw(2, 11, a, b);
        
        print_str("MULT MOD: a=");
        print_int(a);
        print_str(", b=");
        print_int(b);
        print_str(", sw=");
        print_int(sw_result);
        print_str(", hw=");
        print_int(hw_result);
        print_str("\n");
        
        if (sw_result != hw_result) {
            print_str("*** FAILED ***\n");
            return 0;
        }
    }
    
    print_str("=======================================================\n");
    print_str("================ TESTING NORMAL REDUCE ===============\n");
    print_str("=======================================================\n");
    
    // Test values for normal reduce
    volatile int reduce_test_values[] = {
        0, 5, 127, 128, 129, 255, 256, 257, 
        500, 1000, 1337, 2048, 4096, 65536,
        431, 421, 542, 52335, 4123, 5421, 531,
        41236, 6345, 526, 764, 745, 432, 1, 4, 8
    };
    
    int num_reduce_tests = sizeof(reduce_test_values) / sizeof(reduce_test_values[0]);
    
    // Test NORMAL REDUCE (any other funct3/funct7 combination)
    for (int i = 0; i < num_reduce_tests; i++) {
        volatile int test_val = reduce_test_values[i];
        volatile int sw_result, hw_result;
        
        // Software: just reduce
        sw_result = test_val % mod;
        
        // Hardware: normal reduce (using different funct3/funct7)
        hw_result = cfu_op_hw(0, 11, test_val, 0);  // Any other combination
        
        print_str("NORMAL REDUCE: val=");
        print_int(test_val);
        print_str(", sw=");
        print_int(sw_result);
        print_str(", hw=");
        print_int(hw_result);
        print_str("\n");
        
        if (sw_result != hw_result) {
            print_str("*** FAILED ***\n");
            return 0;
        }
    }
    
    print_str("=======================================================\n");
    print_str("=================== ALL TESTS PASSED =================\n");
    print_str("=======================================================\n");
    return 1;
}