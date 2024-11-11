#include <cassert>
#include "ram.h"
#include "bit_vector.h"

using namespace std;

// input: vector x of equal-length bit vectors x[0],...,x[N-1]
// input: integer L between 0 and N-1
// input: integer H between L+1 and N
// input: bit vector i specifying I = i[0]+2*i[1]+...+2^(ibits-1)*i[ibits-1]
// output: x[L+J] for some J with 0 <= J < H-L
// satisfying J=I if 0 <= I < H-L; no constraints if I >= H-L
const vector<bit> ram_read(
  const vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits)
{
  if (H <= L) return vector<bit>{};
  if (H == L+1) return x.at(L);

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }
  // now H-L <= split+split
  // and split == 2^splitpos

  // conventional RAM circuit:
  // use i[0:splitpos] to look up entry in x[L:L+split]
  //   result0 = x[L+(I mod split)]
  // use i[0:splitpos] to look up entry in x[L+split:H], called result1
  //   result1 = x[L+split+(I mod split)] if L+split+(I mod split) < H
  // multiplex according to i[splitpos]

  // why this works:
  // assume I < H-L; then I < split+split
  // case 0: i[splitpos] is 0
  //   then I < split
  //   so result0 = x[L+I], and we'll select result0
  // case 1: i[splitpos] is 1
  //   then split <= I < H-L <= split+split
  //   so split+(I mod split) = I <= H-L
  //   so result1 = x[L+I], and we'll select result1

  if (ibits <= splitpos) {
    // different case: definitely want result from x[L:L+split], no multiplexing
    return ram_read(x,L,L+split,i,ibits);
  }

  vector<bit> result0 = ram_read(x,L,L+split,i,splitpos);
  vector<bit> result1 = ram_read(x,L+split,H,i,splitpos);

  assert(result0.size() == result1.size());

  bit isplit = i.at(splitpos);

  vector<bit> result{};
  for (bigint r = 0;r < result0.size();++r) {
    bit x0 = result0.at(r);
    bit x1 = result1.at(r);
    result.push_back(isplit.mux(x0,x1));
  }

  return result;
}

const vector<bit> ram_read(
  const vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i)
{
	return ram_read(x, L, H, i, i.size());
}

const vector<bit> ram_read(
  const vector<std::vector<bit>> &x,
  const vector<bit> &i)
{
	return ram_read(x, 0, x.size(), i);
}

// same as ram_read above but only returns the jth bit
const bit ram_read(
  const vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits,
  bigint j)
{
  if (H <= L) return bit(0);
  if (H == L+1) return x.at(L).at(j);

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }

  if (ibits <= splitpos) {
    return ram_read(x,L,L+split,i,ibits,j);
  }

  bit result0 = ram_read(x,L,L+split,i,splitpos,j);
  bit result1 = ram_read(x,L+split,H,i,splitpos,j);

  bit isplit = i.at(splitpos);
  bit result = isplit.mux(result0,result1);

  return result;
}

// same as ram_read above but x is a vector of bits
const bit ram_read(
  const vector<bit> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits)
{
  if (H <= L) return bit(0);
  if (H == L+1) return x.at(L);

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }

  if (ibits <= splitpos) {
    return ram_read(x,L,L+split,i,ibits);
  }

  bit result0 = ram_read(x,L,L+split,i,splitpos);
  bit result1 = ram_read(x,L+split,H,i,splitpos);

  bit isplit = i.at(splitpos);
  bit result = isplit.mux(result0,result1);

  return result;
}

const bit ram_read(
  const vector<bit> &x,
  const vector<bit> &i)
{
	return ram_read(x, 0, x.size(), i, i.size());
}

// input: vector x of equal-length bit vectors x[0],...,x[N-1]
// input: integer L between 0 and N-1
// input: integer H between L+1 and N
// input: bit vector i specifying I = i[0]+2*i[1]+...+2^(ibits-1)*i[ibits-1]
// input: bit vector data of length the same as any x[*]
// input: bit b with default value 0; only used for recursive calls.
// write data to x[L+J] for some J with 0 <= J < H-L
// satisfying J=I if 0 <= I < H-L; no constraints if I >= H-L
void ram_write(
  vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits,
  const vector<bit> &data,
  bit b, bool top)
{
  assert (x.at(0).size() == data.size());

  if (H <= L) return;
  if (H == L+1) {
    if (top)
      x.at(L) = data;
    else
      for (bigint r = 0;r < data.size();++r)
        x.at(L).at(r) = b.mux(data.at(r), x.at(L).at(r));
    return;
  }

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }

  if (ibits <= splitpos) {
    return ram_write(x,L,L+split,i,ibits,data,b,top);
  }

  bit isplit = i.at(splitpos);

  ram_write(x,L,L+split,i,splitpos,data, top ?  isplit :  (b | isplit), 0);
  ram_write(x,L+split,H,i,splitpos,data, top ? ~isplit : b.orn(isplit), 0);
}

void ram_write(
  vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  const vector<bit> &data)
{
	ram_write(x, L, H, i, i.size(), data);
}

void ram_write(
  vector<std::vector<bit>> &x,
  const vector<bit> &i,
  const vector<bit> &data)
{
	ram_write(x, 0, x.size(), i, data);
}

const vector<bit> ram_read_write(
  vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits,
  vector<bit> &data,
  bit b, bool top)
{
  assert (x.at(0).size() == data.size());

  if (H <= L) return vector<bit>{};

  if (H == L+1) {
    vector<bit> v = x.at(L);
    if (top)
      x.at(L) = data;
    else
      bit_vector_mux(x.at(L), data, x.at(L), b);
    return v;
  }

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }

  if (ibits <= splitpos) {
    return ram_read_write(x,L,L+split,i,ibits,data,b, top);
  }

  bit isplit = i.at(splitpos);

  vector<bit> result0 = ram_read_write(x,L,L+split,i,splitpos,data, top ?  isplit :  (b | isplit), 0);
  vector<bit> result1 = ram_read_write(x,L+split,H,i,splitpos,data, top ? ~isplit : b.orn(isplit), 0);

  assert(result0.size() == result1.size());

  vector<bit> result{};
  for (bigint r = 0;r < result0.size();++r) {
    bit x0 = result0.at(r);
    bit x1 = result1.at(r);
    result.push_back(isplit.mux(x0,x1));
  }

  return result;
}

const vector<bit> ram_read_write(
  vector<std::vector<bit>> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  vector<bit> &data)
{
	return ram_read_write(x, L, H, i, i.size(), data);
}

const vector<bit> ram_read_write(
  vector<std::vector<bit>> &x,
  const vector<bit> &i,
  vector<bit> &data)
{
	return ram_read_write(x, 0, x.size(), i, data);
}


void ram_write(
  vector<bit> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  bigint ibits,
  const bit data,
  bit b, 
  bool top)
{
  if (H <= L) return;
  if (H == L+1) 
  {
    x.at(L) = b.mux(data, x.at(L));
    return;
  }

  bigint splitpos = 0;
  bigint split = 1;
  while (L+split < H-split) {
    splitpos += 1;
    split *= 2;
  }

  if (ibits <= splitpos) {
    return ram_write(x,L,L+split,i,ibits,data,b,top);
  }

  bit isplit = i.at(splitpos);

  ram_write(x,L,L+split,i,splitpos,data, top ?  isplit :  (b | isplit), 0);
  ram_write(x,L+split,H,i,splitpos,data, top ? ~isplit : b.orn(isplit), 0);

  return;
}

void ram_write(
  vector<bit> &x,
  bigint L,
  bigint H,
  const vector<bit> &i,
  const bit data)
{
	ram_write(x, L, H, i, i.size(), data);
}

void ram_write(
  vector<bit> &x,
  const vector<bit> &i,
  const bit data)
{
	ram_write(x, 0, x.size(), i, data);
}

