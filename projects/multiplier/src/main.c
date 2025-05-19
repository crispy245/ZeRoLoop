#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"

void test_and_print(int a, int b) {
    print_str("Test case: ");
    print_int(a);
    print_str(" * ");
    print_int(b);
    print_str(" = ");
    
    int hw_result = cfu_op0_hw(2, a, b);
    print_int(hw_result);

    print_str(" (Expected: ");
    int sw_result = a*b;
    print_int(sw_result);    
    print_str(")");

    if(hw_result != sw_result){
        print_str("!!!ERROR!!!");
    }

    print_char('\n');

}

int main() {
    
    print_str("Extended test cases:\n");

    // Test with negative numbers
    test_and_print(-5, 5);          
    test_and_print(326,3329);
    test_and_print(5, -5);          
    // test_and_print(-5, -5);      
    // test_and_print(-127, 127);        
    // test_and_print(255, -255);     
    // test_and_print(-32768, 1);       
    // test_and_print(1, -32768);       
    // test_and_print(-32768, -1);      
    
    // // Test with positive numbers
    // test_and_print(5, 10);           
    // test_and_print(127, 127);        
    // test_and_print(255, 255);        
    // test_and_print(32767, 1);        
    // test_and_print(65535, 2);        
    
    // // Test with zeros
    // test_and_print(0, 0);            
    // test_and_print(42, 0);           
    // test_and_print(0, 42);           
    
    
    // // Test with powers of 2 (special case for binary multiplication)
    // test_and_print(1, 1024);         
    // test_and_print(2, 2048);         
    // test_and_print(4096, 16);        
    
    // // Test with values that cause overflow in 32-bit result
    // test_and_print(65535, 65535);    
    // test_and_print(131071, 32767);   
    
    // // Test with prime numbers
    // test_and_print(17, 23);          
    // test_and_print(101, 199);        
    
    // // Random values
    // test_and_print(-123, 456);        
    // test_and_print(7890, 1234);      
    // test_and_print(420, -420);  
    // test_and_print(69, -69);  
    
    return 0;
}