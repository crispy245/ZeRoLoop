#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include <vector>

#include "bigint.h"
#include "bit.h"
#include "util.h"

using namespace std;

static inline vector<bit> bit_vector_extract(const vector<bit> &v, bigint start, bigint end)
{
	assert(end >= start);

	vector<bit> ret(0);

	for (bigint i = start; i < end; i++)
		ret.push_back(v.at(i));

	return ret;
}

static inline void bit_vector_clear(vector<bit> &v)
{
	for (bigint i = 0; i < v.size(); i++)
		v.at(i) = bit(0);
}

static inline void bit_vector_mux(vector<bit> &dest, 
                                  vector<bit> &src,
                                  bit b)
{
	assert(dest.size() == src.size());

	for (bigint i = 0; i < dest.size(); i++)
		dest.at(i) = b.mux(dest.at(i), src.at(i));
}

static inline void bit_vector_mux(vector<bit> &dest, 
                             vector<bit> &src0, 
                             vector<bit> &src1,
                             bit b)
{
	assert(dest.size() == src0.size() && src1.size() == src0.size());

	for (bigint i = 0; i < dest.size(); i++)
		dest.at(i) = b.mux(src0.at(i), src1.at(i));
}

static inline bit bit_vector_compare(const vector<bit> &v, const vector<bit> &w)
{
	assert(v.size() == w.size());
	if (v.size() == 0) return bit(0);

	bit ret = v.at(0) ^ w.at(0);
	for (bigint i = 1; i < v.size(); i++)
		ret |= v.at(i) ^ w.at(i);

	return ret;
}

static inline bit bit_vector_integer_compare(const vector<bit> &v, const vector<bit> &w)
{
	bigint i;
	bit ret;

	for (i = 0; i < min(v.size(), w.size()); i++)
		ret |= v.at(i) ^ w.at(i);

	for (; i < max(v.size(), w.size()); i++)
		ret |= (v.size() > w.size()) ? v.at(i) : w.at(i);
		
	return ret;
}

static inline vector<bit> bit_vector_from_integer(bigint n)
{
	vector<bit> ret(nbits(n));

	for (bigint i = 0; i < ret.size(); i++)
		ret.at(i) = n.bit(i);

	return ret;
}



static inline vector<bit> bit_vector_from_integer(bigint n, bigint len, bool flip = 0)
{
	assert(nbits(n) <= len);

	vector<bit> ret(len);

	for (bigint i = 0; i < ret.size(); i++)
        {
                if (flip == 0) ret.at(i) = n.bit(i);
                else           ret.at(i) = n.bit(i) ^ 1;
        }

	return ret;
}

static inline vector<bit> bit_vector_first_one(vector<bit> &v)
{
        bigint n = v.size();
        bigint splitpos = 0;
        bigint split = 1;

        vector<bit> idx(nbits(n-1));

        if (n == 1) idx.at(0) = bit(0);

        while (split*2 < n)
        {
                splitpos += 1;
                split *= 2;
        }

        for (bigint i = splitpos; i >= 0; i--)
        {
                bit b = v.at(0);
                for (bigint j = 1; j < split; j++)
                        b |= v.at(j);

                idx.at(i) = ~b;
                for (bigint j = 0; j < split; j++)
                        if (j + split < n)
                                v.at(j) = b.mux(v.at(j + split), v.at(j));

                split >>= 1;
        }

        return idx;
}

static inline void bit_vector_ixor(vector<bit> &v, 
                                   const vector<bit> &w)
{
	assert(v.size() == w.size());

	for (bigint i = 0; i < v.size(); i++)
		v.at(i) = v.at(i) ^ w.at(i);	
}

static inline vector<bit> bit_vector_xor(const vector<bit> &v, 
                                         const vector<bit> &w)
{
	assert(v.size() == w.size());

	vector<bit> ret(v.size());

	for (bigint i = 0; i < v.size(); i++)
		ret.at(i) = v.at(i) ^ w.at(i);	

	return ret;
}

static inline bit bit_vector_and_bits(const vector<bit> &v)
{
        if (v.size() == 0)
                return bit(0);

        bit ret = v.at(0);

        for (bigint i = 1; i < v.size(); i++)
                ret &= v.at(i);

        return ret;
}

static inline bit bit_vector_or_bits(const vector<bit> &v)
{
	if (v.size() == 0)
		return bit(0);

	bit ret = v.at(0);

	for (bigint i = 1; i < v.size(); i++)
		ret |= v.at(i);	

	return ret;
}

static inline bit bit_vector_iszero(const vector<bit> &v)
{
	if (v.size() == 0)
		return bit(1);

	bit ret = v.at(0);
	for (bigint i = 1; i < v.size(); i++)
		ret |= v.at(i);	

	return ~ret;
}

static inline void bit_vector_cswap(const bit c, vector<bit> &v, vector<bit> &w)
{
	assert(v.size() == w.size());

	for (bigint i = 0; i < v.size(); i++)
		c.cswap(v.at(i), w.at(i));
}

static inline void bit_queue_insert(vector<bit> &q, 
                                    bit v,
                                    bit b)
{
	bigint i = q.size();
	i -= 2;

	for (; i >= 0; i--)
		q.at(i+1) = b.mux(q.at(i+1), q.at(i));

	q.at(0) = b.mux(q.at(0), v);
}

// same as bit_queue but inserts only 1, so shape is 111...000...
static inline void bit_queue1_insert(vector<bit> &q, 
                                     bit b)
{
	bigint i = q.size();
	i -= 2;

	if (bit_mux_cost < bit_or_cost+bit_and_cost)
		for (; i >= 0; i--)
			q.at(i+1) = b.mux(q.at(i+1), q.at(i));
	else
		for (; i >= 0; i--)
			q.at(i+1) |= b&q.at(i);

	q.at(0) |= b;
}

static inline void bit_vector_queue_insert(vector<vector<bit>> &q, 
                                           vector<bit> &v,
                                           bit b)
{
	bigint i = q.size();
	i -= 2;

	for (; i >= 0; i--)
		bit_vector_mux(q.at(i+1), q.at(i), b);

	bit_vector_mux(q.at(0), v, b);
}

#endif
