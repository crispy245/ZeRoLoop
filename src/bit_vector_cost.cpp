#include <cassert>
#include "bit_cost.h"
#include "bit_vector_cost.h"

bigint bit_vector_iszero_cost(bigint L)
{
  return L; // XXX: can switch to nonzero to save 1 op
}

bigint bit_vector_compare_cost(bigint n)
{
  if (n <= 0) return 0;
  return 2*n-1;
}

bigint bit_queue1_insert_cost(bigint q)
{
  assert(q > 0);
  if (bit_mux_cost < bit_or_cost+bit_and_cost)
    return bit_or_cost+bit_mux_cost*(q-1); // q.at(i+1) = b.mux(q.at(i+1), q.at(i)); ... q.at(0) |= v;
  return bit_or_cost+(bit_or_cost+bit_and_cost)*(q-1); // q.at(i+1) |= b&q.at(i); ... q.at(0) |= v;
}

bigint bit_vector_integer_compare_cost(bigint n,bigint m)
{
  return n+m; // XXX: can speed up circuit
}

bigint bit_vector_first_one_cost(bigint N)
{
  bigint splitpos = 0;
  bigint split = 1;
  bigint result;

  while (split*2 < N) {
    ++splitpos;
    split *= 2;
  }
  for (bigint i = splitpos;i >= 0;--i) {
    result += split-1; // b |= v.at(j);
    result += 1; // idx.at(i) = ~b
    for (bigint j = 0;j < split;++j)
      if (j+split < N)
        result += bit_mux_cost; // v.at(j) = b.mux(v.at(j + split), v.at(j));
    split >>= 1;
  }

  return result;
}
