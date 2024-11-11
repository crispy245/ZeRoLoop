#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include "bigint_mpz.h"

#define BIT_LIMIT 1073741824

using namespace std;

mpz_ptr bigint::unsafe_get_mpz_t(void)
{
  return x;
}

bool bigint::bit(bigint e)
{
  if (e < 0) return 0;
  assert(e < BIT_LIMIT); // XXX
  return mpz_tstbit(x,e);
}

bigint::operator bool() const { return mpz_sgn(x) != 0; }

bigint::operator int() const { assert(mpz_fits_sint_p(x)); return mpz_get_si(x); }
bigint::operator long() const { assert(mpz_fits_slong_p(x)); return mpz_get_si(x); }
bigint::operator unsigned int() const { assert(mpz_fits_uint_p(x)); return mpz_get_ui(x); }
bigint::operator unsigned long() const { assert(mpz_fits_ulong_p(x)); return mpz_get_ui(x); }

bigint::operator unsigned long long() const
{
  assert(mpz_sgn(x) >= 0);
  assert(mpz_sizeinbase(x,256) <= sizeof(long long));

  unsigned char r[sizeof(long long)];
  for (long long i = 0;i < sizeof r;++i) r[i] = 0;
  mpz_export(r,0,-1,1,0,0,x);
  unsigned long long result = 0;
  for (long long i = (sizeof r)-1;i >= 0;--i)
    result = result*256+r[i];
  return result;
}

bigint::operator long long() const
{
  mpz_t top;
  mpz_t bot;
  mpz_init_set_ui(top,1);
  mpz_init_set_si(bot,-1);
  mpz_mul_2exp(top,top,8*sizeof(long long)-1);
  mpz_mul_2exp(bot,bot,8*sizeof(long long)-1);
  assert(mpz_cmp(x,top) <= 0);
  assert(mpz_cmp(x,bot) >= 0);
  mpz_clear(top);
  mpz_clear(bot);

  unsigned char r[sizeof(long long)];
  for (long long i = 0;i < sizeof r;++i) r[i] = 0;
  mpz_export(r,0,-1,1,0,0,x);
  unsigned long long result = 0;
  for (long long i = (sizeof r)-1;i >= 0;--i)
    result = result*256+r[i];
  if (mpz_sgn(x) < 0) result = -result;
  return result;
}

bigint& operator+=(bigint &a,const bigint &b)
{
  mpz_add(a.x,a.x,b.x);
  return a;
}

bigint& operator+=(bigint &a,int b) { return a += bigint(b); }

bigint operator-(const bigint &a)
{
  bigint result;
  mpz_neg(result.x,a.x);
  return result;
}

bigint& operator-=(bigint &a,const bigint &b)
{
  mpz_sub(a.x,a.x,b.x);
  return a;
}

bigint& operator*=(bigint &a,const bigint &b)
{
  mpz_mul(a.x,a.x,b.x);
  return a;
}

bigint& operator/=(bigint &a,const bigint &b)
{
  mpz_fdiv_q(a.x,a.x,b.x);
  return a;
}

bigint& operator%=(bigint &a,const bigint &b)
{
  mpz_fdiv_r(a.x,a.x,b.x);
  return a;
}

bigint& operator<<=(bigint &a,const bigint &b)
{
  return a = a<<b;
}

bigint& operator>>=(bigint &a,const bigint &b)
{
  return a = a>>b;
}

bool operator==(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) == 0; }
bool operator==(const bigint &a,int b) { return mpz_cmp_si(a.x,b) == 0; }
bool operator==(const bigint &a,long b) { return mpz_cmp_si(a.x,b) == 0; }
bool operator==(const bigint &a,long long b) { return a == bigint(b); }
bool operator==(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) == 0; }
bool operator==(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) == 0; }
bool operator==(const bigint &a,unsigned long long b) { return a == bigint(b); }
bool operator==(int b,const bigint &a) { return mpz_cmp_si(a.x,b) == 0; }
bool operator==(long b,const bigint &a) { return mpz_cmp_si(a.x,b) == 0; }
bool operator==(long long b,const bigint &a) { return a == bigint(b); }
bool operator==(unsigned int b,const bigint &a) { return mpz_cmp_ui(a.x,b) == 0; }
bool operator==(unsigned long b,const bigint &a) { return mpz_cmp_ui(a.x,b) == 0; }
bool operator==(unsigned long long b,const bigint &a) { return a == bigint(b); }

bool operator!=(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) != 0; }
bool operator!=(const bigint &a,int b) { return mpz_cmp_si(a.x,b) != 0; }
bool operator!=(const bigint &a,long b) { return mpz_cmp_si(a.x,b) != 0; }
bool operator!=(const bigint &a,long long b) { return a != bigint(b); }
bool operator!=(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) != 0; }
bool operator!=(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) != 0; }
bool operator!=(const bigint &a,unsigned long long b) { return a != bigint(b); }
bool operator!=(int b,const bigint &a) { return mpz_cmp_si(a.x,b) != 0; }
bool operator!=(long b,const bigint &a) { return mpz_cmp_si(a.x,b) != 0; }
bool operator!=(long long b,const bigint &a) { return a != bigint(b); }
bool operator!=(unsigned int b,const bigint &a) { return mpz_cmp_ui(a.x,b) != 0; }
bool operator!=(unsigned long b,const bigint &a) { return mpz_cmp_ui(a.x,b) != 0; }
bool operator!=(unsigned long long b,const bigint &a) { return a != bigint(b); }

bool operator<(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) < 0; }
bool operator<(const bigint &a,int b) { return mpz_cmp_si(a.x,b) < 0; }
bool operator<(const bigint &a,long b) { return mpz_cmp_si(a.x,b) < 0; }
bool operator<(const bigint &a,long long b) { return a < bigint(b); }
bool operator<(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) < 0; }
bool operator<(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) < 0; }
bool operator<(const bigint &a,unsigned long long b) { return a < bigint(b); }
bool operator<(int b,const bigint &a) { return a > b; }
bool operator<(long b,const bigint &a) { return a > b; }
bool operator<(long long b,const bigint &a) { return a > b; }
bool operator<(unsigned int b,const bigint &a) { return a > b; }
bool operator<(unsigned long b,const bigint &a) { return a > b; }
bool operator<(unsigned long long b,const bigint &a) { return a > b; }

bool operator<=(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) <= 0; }
bool operator<=(const bigint &a,int b) { return mpz_cmp_si(a.x,b) <= 0; }
bool operator<=(const bigint &a,long b) { return mpz_cmp_si(a.x,b) <= 0; }
bool operator<=(const bigint &a,long long b) { return a <= bigint(b); }
bool operator<=(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) <= 0; }
bool operator<=(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) <= 0; }
bool operator<=(const bigint &a,unsigned long long b) { return a <= bigint(b); }
bool operator<=(int b,const bigint &a) { return a >= b; }
bool operator<=(long b,const bigint &a) { return a >= b; }
bool operator<=(long long b,const bigint &a) { return a >= b; }
bool operator<=(unsigned int b,const bigint &a) { return a >= b; }
bool operator<=(unsigned long b,const bigint &a) { return a >= b; }
bool operator<=(unsigned long long b,const bigint &a) { return a >= b; }

bool operator>(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) > 0; }
bool operator>(const bigint &a,int b) { return mpz_cmp_si(a.x,b) > 0; }
bool operator>(const bigint &a,long b) { return mpz_cmp_si(a.x,b) > 0; }
bool operator>(const bigint &a,long long b) { return a > bigint(b); }
bool operator>(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) > 0; }
bool operator>(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) > 0; }
bool operator>(const bigint &a,unsigned long long b) { return a > bigint(b); }
bool operator>(int b,const bigint &a) { return a < b; }
bool operator>(long b,const bigint &a) { return a < b; }
bool operator>(long long b,const bigint &a) { return a < b; }
bool operator>(unsigned int b,const bigint &a) { return a < b; }
bool operator>(unsigned long b,const bigint &a) { return a < b; }
bool operator>(unsigned long long b,const bigint &a) { return a < b; }

bool operator>=(const bigint &a,const bigint &b) { return mpz_cmp(a.x,b.x) >= 0; }
bool operator>=(const bigint &a,int b) { return mpz_cmp_si(a.x,b) >= 0; }
bool operator>=(const bigint &a,long b) { return mpz_cmp_si(a.x,b) >= 0; }
bool operator>=(const bigint &a,long long b) { return a >= bigint(b); }
bool operator>=(const bigint &a,unsigned int b) { return mpz_cmp_ui(a.x,b) >= 0; }
bool operator>=(const bigint &a,unsigned long b) { return mpz_cmp_ui(a.x,b) >= 0; }
bool operator>=(const bigint &a,unsigned long long b) { return a >= bigint(b); }
bool operator>=(int b,const bigint &a) { return a <= b; }
bool operator>=(long b,const bigint &a) { return a <= b; }
bool operator>=(long long b,const bigint &a) { return a <= b; }
bool operator>=(unsigned int b,const bigint &a) { return a <= b; }
bool operator>=(unsigned long b,const bigint &a) { return a <= b; }
bool operator>=(unsigned long long b,const bigint &a) { return a <= b; }

bigint operator&(const bigint &a,const bigint &b)
{
  bigint result;
  mpz_and(result.x,a.x,b.x);
  return result;
}

bigint operator&(const bigint &a,int b) { return a&bigint(b); }

bigint operator+(const bigint &a,const bigint &b) { bigint result; mpz_add(result.x,a.x,b.x); return result; }
bigint operator+(const bigint &a,int b) { return a+bigint(b); }
bigint operator+(const bigint &a,unsigned int b) { bigint result; mpz_add_ui(result.x,a.x,b); return result; }
bigint operator+(const bigint &a,long b) { return a+bigint(b); }
bigint operator+(const bigint &a,unsigned long b) { bigint result; mpz_add_ui(result.x,a.x,b); return result; }
bigint operator+(const bigint &a,long long b) { return a+bigint(b); }
bigint operator+(const bigint &a,unsigned long long b) { return a+bigint(b); }
bigint operator+(int b,const bigint &a) { return a+bigint(b); }
bigint operator+(unsigned int b,const bigint &a) { bigint result; mpz_add_ui(result.x,a.x,b); return result; }
bigint operator+(long b,const bigint &a) { return a+bigint(b); }
bigint operator+(unsigned long b,const bigint &a) { bigint result; mpz_add_ui(result.x,a.x,b); return result; }
bigint operator+(long long b,const bigint &a) { return a+bigint(b); }
bigint operator+(unsigned long long b,const bigint &a) { return a+bigint(b); }

bigint operator*(const bigint &a,const bigint &b) { bigint result; mpz_mul(result.x,a.x,b.x); return result; }
bigint operator*(const bigint &a,int b) { return a*bigint(b); }
bigint operator*(const bigint &a,unsigned int b) { bigint result; mpz_mul_ui(result.x,a.x,b); return result; }
bigint operator*(const bigint &a,long b) { return a*bigint(b); }
bigint operator*(const bigint &a,unsigned long b) { bigint result; mpz_mul_ui(result.x,a.x,b); return result; }
bigint operator*(const bigint &a,long long b) { return a*bigint(b); }
bigint operator*(const bigint &a,unsigned long long b) { return a*bigint(b); }
bigint operator*(int b,const bigint &a) { return a*bigint(b); }
bigint operator*(unsigned int b,const bigint &a) { bigint result; mpz_mul_ui(result.x,a.x,b); return result; }
bigint operator*(long b,const bigint &a) { return a*bigint(b); }
bigint operator*(unsigned long b,const bigint &a) { bigint result; mpz_mul_ui(result.x,a.x,b); return result; }
bigint operator*(long long b,const bigint &a) { return a*bigint(b); }
bigint operator*(unsigned long long b,const bigint &a) { return a*bigint(b); }

bigint operator-(const bigint &a,const bigint &b) { bigint result; mpz_sub(result.x,a.x,b.x); return result; }
bigint operator-(const bigint &a,int b) { return a-bigint(b); }
bigint operator-(const bigint &a,unsigned int b) { bigint result; mpz_sub_ui(result.x,a.x,b); return result; }
bigint operator-(const bigint &a,long b) { return a-bigint(b); }
bigint operator-(const bigint &a,unsigned long b) { bigint result; mpz_sub_ui(result.x,a.x,b); return result; }
bigint operator-(const bigint &a,long long b) { return a-bigint(b); }
bigint operator-(const bigint &a,unsigned long long b) { return a-bigint(b); }
bigint operator-(int b,const bigint &a) { return bigint(b)-a; }
bigint operator-(unsigned int b,const bigint &a) { return bigint(b)-a; }
bigint operator-(long b,const bigint &a) { return bigint(b)-a; }
bigint operator-(unsigned long b,const bigint &a) { return bigint(b)-a; }
bigint operator-(long long b,const bigint &a) { return bigint(b)-a; }
bigint operator-(unsigned long long b,const bigint &a) { return bigint(b)-a; }

bigint operator/(const bigint &a,const bigint &b) { bigint result; mpz_fdiv_q(result.x,a.x,b.x); return result; }
bigint operator/(const bigint &a,int b) { return a/bigint(b); }
bigint operator/(const bigint &a,unsigned int b) { return a/bigint(b); }
bigint operator/(const bigint &a,long b) { return a/bigint(b); }
bigint operator/(const bigint &a,unsigned long b) { return a/bigint(b); }
bigint operator/(const bigint &a,long long b) { return a/bigint(b); }
bigint operator/(const bigint &a,unsigned long long b) { return a/bigint(b); }
bigint operator/(int b,const bigint &a) { return bigint(b)/a; }
bigint operator/(unsigned int b,const bigint &a) { return bigint(b)/a; }
bigint operator/(long b,const bigint &a) { return bigint(b)/a; }
bigint operator/(unsigned long b,const bigint &a) { return bigint(b)/a; }
bigint operator/(long long b,const bigint &a) { return bigint(b)/a; }
bigint operator/(unsigned long long b,const bigint &a) { return bigint(b)/a; }

bigint operator%(const bigint &a,const bigint &b) { bigint result; mpz_fdiv_r(result.x,a.x,b.x); return result; }
bigint operator%(const bigint &a,int b) { return a%bigint(b); }
bigint operator%(const bigint &a,unsigned int b) { return a%bigint(b); }
bigint operator%(const bigint &a,long b) { return a%bigint(b); }
bigint operator%(const bigint &a,unsigned long b) { return a%bigint(b); }
bigint operator%(const bigint &a,long long b) { return a%bigint(b); }
bigint operator%(const bigint &a,unsigned long long b) { return a%bigint(b); }
bigint operator%(int b,const bigint &a) { return bigint(b)%a; }
bigint operator%(unsigned int b,const bigint &a) { return bigint(b)%a; }
bigint operator%(long b,const bigint &a) { return bigint(b)%a; }
bigint operator%(unsigned long b,const bigint &a) { return bigint(b)%a; }
bigint operator%(long long b,const bigint &a) { return bigint(b)%a; }
bigint operator%(unsigned long long b,const bigint &a) { return bigint(b)%a; }

bigint &operator++(bigint &a)
{
  mpz_add_ui(a.x,a.x,1);
  return a;
}

bigint operator++(bigint &a,int)
{
  bigint result = a;
  mpz_add_ui(a.x,a.x,1);
  return result;
}

bigint& operator--(bigint &a)
{
  mpz_sub_ui(a.x,a.x,1);
  return a;
}

bigint operator--(bigint &a,int)
{
  bigint result = a;
  mpz_sub_ui(a.x,a.x,1);
  return result;
}

bigint operator<<(const bigint &a,const bigint &b)
{
  bigint result;
  assert(mpz_fits_slong_p(b.x)); // XXX
  long shiftcount = mpz_get_si(b.x);
  assert(shiftcount >= 0);
  assert(shiftcount <= BIT_LIMIT); // XXX
  mpz_mul_2exp(result.x,a.x,shiftcount);
  return result;
}

bigint operator<<(const bigint &a,int b) { return a<<(bigint) b; }
bigint operator<<(const bigint &a,long b) { return a<<(bigint) b; }
bigint operator<<(const bigint &a,long long b) { return a<<(bigint) b; }
bigint operator<<(const bigint &a,unsigned int b) { return a<<(bigint) b; }
bigint operator<<(const bigint &a,unsigned long b) { return a<<(bigint) b; }
bigint operator<<(const bigint &a,unsigned long long b) { return a<<(bigint) b; }
bigint operator<<(int a,const bigint &b) { return bigint(a)<<b; }
bigint operator<<(long a,const bigint &b) { return bigint(a)<<b; }
bigint operator<<(long long a,const bigint &b) { return bigint(a)<<b; }
bigint operator<<(unsigned int a,const bigint &b) { return bigint(a)<<b; }
bigint operator<<(unsigned long a,const bigint &b) { return bigint(a)<<b; }
bigint operator<<(unsigned long long a,const bigint &b) { return bigint(a)<<b; }

bigint operator>>(const bigint &a,const bigint &b)
{
  bigint bits(mpz_sizeinbase(a.x,2));
  if (b > bits) {
    if (a < 0) return -1;
    return 0;
  }
  bigint result;
  long shiftcount = mpz_get_si(b.x);
  assert(shiftcount >= 0);
  assert(shiftcount <= BIT_LIMIT);
  mpz_fdiv_q_2exp(result.x,a.x,shiftcount);
  return result;
}

bigint operator>>(const bigint &a,int b) { return a>>(bigint) b; }
bigint operator>>(const bigint &a,long b) { return a>>(bigint) b; }
bigint operator>>(const bigint &a,long long b) { return a>>(bigint) b; }
bigint operator>>(const bigint &a,unsigned int b) { return a>>(bigint) b; }
bigint operator>>(const bigint &a,unsigned long b) { return a>>(bigint) b; }
bigint operator>>(const bigint &a,unsigned long long b) { return a>>(bigint) b; }
bigint operator>>(int a,const bigint &b) { return bigint(a)>>b; }
bigint operator>>(long a,const bigint &b) { return bigint(a)>>b; }
bigint operator>>(long long a,const bigint &b) { return bigint(a)>>b; }
bigint operator>>(unsigned int a,const bigint &b) { return bigint(a)>>b; }
bigint operator>>(unsigned long a,const bigint &b) { return bigint(a)>>b; }
bigint operator>>(unsigned long long a,const bigint &b) { return bigint(a)>>b; }

std::string bigint::get_str(void) const
{
  char *s = mpz_get_str(0,10,x);
  assert(s);
  std::string result(s);
  free(s);
  return result;
}

ostream &operator<<(ostream &o,const bigint &a)
{
  return o << a.get_str();
}

static map<pair<bigint,bigint>,bigint> binomial_cache;

bigint binomial(bigint n,bigint k)
{
  if (k < 0) return 0;
  if (k == 0) return 1;
  if (n >= 0) if (k > n) return 0;
  pair<bigint,bigint> nk = make_pair(n,k);
  if (binomial_cache.count(nk))
    return binomial_cache[nk];
  bigint result = binomial(n-1,k-1)*n/k;
  binomial_cache[nk] = result;
  return result;
}

bigint nbits(bigint bits)
{
  if (bits <= 0) return 1;
  bigint result = 0;
  while (bits > 0) {
    ++result;
    bits >>= 1;
  }
  return result;
}

bigint nbits0(bigint bits)
{
  if (bits <= 0) return 0;
  return nbits(bits);
}
