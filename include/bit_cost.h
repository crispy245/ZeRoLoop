#ifndef bit_cost_h
#define bit_cost_h

// rule: any function of 2 bits costs 1
#define bit_not_cost 1
#define bit_xor_cost 1
#define bit_and_cost 1
#define bit_or_cost 1
#define bit_xnor_cost 1
#define bit_andn_cost 1
#define bit_nand_cost 1
#define bit_orn_cost 1
#define bit_nor_cost 1
#define bit_mux_cost 3
#define bit_cswap_cost 4

// alternative, more hardware-oriented:
// not=2, nand=3, nor=3, and=4, or=4, mux=7, etc.
// but to really match hardware also need to account for wiring

#endif
