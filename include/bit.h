#ifndef bit_h
#define bit_h

#include "bigint.h"
#include "bit_cost.h"

#include <cassert>
#include <bitset>
#include <vector>

#define bit_slicing 64

enum bit_ops_selector {
  bit_ops_cost,
  bit_ops_not,
  bit_ops_xor,
  bit_ops_and,
  bit_ops_or,
  bit_ops_xnor,
  bit_ops_andn,
  bit_ops_nand,
  bit_ops_orn,
  bit_ops_nor,
  bit_ops_mux,
  bit_ops_cswap,
} ;

const std::vector<bit_ops_selector> bit_ops_selectors = {
  bit_ops_cost,
  bit_ops_not,
  bit_ops_xor,
  bit_ops_and,
  bit_ops_or,
  bit_ops_xnor,
  bit_ops_andn,
  bit_ops_nand,
  bit_ops_orn,
  bit_ops_nor,
  bit_ops_mux,
  bit_ops_cswap,
} ;

class bit {
  static bigint cost;
  static bigint numnot;
  static bigint numxor;
  static bigint numand;
  static bigint numor;
  static bigint numxnor;
  static bigint numandn;
  static bigint numnand;
  static bigint numorn;
  static bigint numnor;
  static bigint nummux;
  static bigint numcswap;
  std::bitset<bit_slicing> b;
public:
  static bigint ops(void) { return cost; }
  static bigint ops(bit_ops_selector t) {
    switch(t) {
      case bit_ops_not: return numnot;
      case bit_ops_xor: return numxor;
      case bit_ops_and: return numand;
      case bit_ops_or: return numor;
      case bit_ops_xnor: return numxnor;
      case bit_ops_andn: return numandn;
      case bit_ops_nand: return numnand;
      case bit_ops_orn: return numorn;
      case bit_ops_nor: return numnor;
      case bit_ops_mux: return nummux;
      case bit_ops_cswap: return numcswap;
      default: return cost;
    }
  }
  static const char *opsname(bit_ops_selector t) {
    switch(t) {
      case bit_ops_not: return "not";
      case bit_ops_xor: return "xor";
      case bit_ops_and: return "and";
      case bit_ops_or: return "or";
      case bit_ops_xnor: return "xnor";
      case bit_ops_andn: return "andn";
      case bit_ops_nand: return "nand";
      case bit_ops_orn: return "orn";
      case bit_ops_nor: return "nor";
      case bit_ops_mux: return "mux";
      case bit_ops_cswap: return "cswap";
      default: return "ops";
    }
  }

  static void clear_all()
  {
    cost = 0;
    numnot = 0;
    numxor = 0;
    numand = 0;
    numor = 0;
    numxnor = 0;
    numandn = 0;
    numnand = 0;
    numorn = 0;
    numnor = 0;
    nummux = 0;
    numcswap = 0;
  }

  std::bitset<bit_slicing> value_vector(void) const { return b; }
  bool value(void) const { return b[0]; }
  void value_assert_eq(const bit &c) { assert(b == c.b); }

  bit(unsigned long i = 0) : b(-(i & 1)) { }
  bit(std::bitset<bit_slicing> x) : b(x) { }

  bit operator~() const { cost += bit_not_cost; ++numnot; return bit(b ^ std::bitset<bit_slicing>(-(unsigned long long) 1)); }
  bit operator^(const bit &c) const { cost += bit_xor_cost; ++numxor; return bit(b ^ c.b); }
  bit operator&(const bit &c) const { cost += bit_and_cost; ++numand; return bit(b & c.b); }
  bit operator|(const bit &c) const { cost += bit_or_cost; ++numor; return bit(b | c.b); }

  bit xnor(const bit &c) const { cost += bit_xnor_cost; ++numxnor; return bit(~(b ^ c.b)); }
  bit andn(const bit &c) const { cost += bit_andn_cost; ++numandn; return bit(b & ~c.b); }
  bit nand(const bit &c) const { cost += bit_nand_cost; ++numnand; return bit(~(b & c.b)); }
  bit orn(const bit &c) const { cost += bit_orn_cost; ++numorn; return bit(b | ~c.b); }
  bit nor(const bit &c) const { cost += bit_nor_cost; ++numnor; return bit(~(b | c.b)); }

  bit mux(const bit &c0,const bit &c1) const { cost += bit_mux_cost; ++nummux; return bit(c0.b ^ (b & (c0.b ^ c1.b))); }
  void cswap(bit &c0,bit &c1) const
  { cost += bit_cswap_cost; ++numcswap;
    bit flip = bit(b & (c0.b ^ c1.b));
    bit t0 = bit(c0.b ^ flip.b); // i.e., this->mux(b0,b1)
    bit t1 = bit(c1.b ^ flip.b); // i.e., this->mux(b1,b0)
    c0 = t0;
    c1 = t1;
  }

  bit operator^=(const bit &c) { *this = *this ^ c; return *this; }
  bit operator&=(const bit &c) { *this = *this & c; return *this; }
  bit operator|=(const bit &c) { *this = *this | c; return *this; }

} ;

#endif
