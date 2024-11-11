#ifndef bit_vector_cost_h
#define bit_vector_cost_h

#include "bigint.h"

bigint bit_vector_iszero_cost(bigint);
bigint bit_vector_compare_cost(bigint);
bigint bit_vector_integer_compare_cost(bigint,bigint);
bigint bit_vector_hamming_weight_cost(bigint);
bigint bit_vector_first_one_cost(bigint);
bigint bit_queue1_insert_cost(bigint);

#endif
