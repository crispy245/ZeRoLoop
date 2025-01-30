#pragma once

#ifndef bigint_h
#define bigint_h

#include <iostream>
#include <string>
#include <gmp.h>
#include "bigint.h"

class bigint {
  mpz_t x;
public:
  ~bigint() { mpz_clear(x); }
  bigint() { mpz_init(x); }
  bigint(mpz_t m) { mpz_init_set(x,m); }
  bigint(const bigint &m) { mpz_init_set(x,m.x); }
  bigint &operator=(const bigint &b) { mpz_set(x,b.x); return *this; }
  bigint(const char *str,int base = 10) { mpz_init_set_str(x,str,base); }
  bigint(std::string s,int base = 10) { mpz_init_set_str(x,s.c_str(),base); }
  bigint(int i) { mpz_init_set_si(x,i); }
  bigint(unsigned int u) { mpz_init_set_si(x,u); }
  bigint(long i) { mpz_init_set_si(x,i); }
  bigint(unsigned long u) { mpz_init_set_ui(x,u); }
  bigint(unsigned long long i)
  {
    mpz_init(x);
    unsigned char r[sizeof(unsigned long long)];
    for (long long j = 0;j < sizeof r;++j)
      r[j] = 255&(i>>(8*j));
    mpz_import(x,8,-1,1,0,0,r);
  }
  bigint(long long i)
  {
    unsigned long long u = i;
    mpz_init(x);
    unsigned char r[sizeof(long long)];
    for (long long j = 0;j < sizeof r;++j)
      r[j] = 255&(u>>(8*j));
    mpz_import(x,8,-1,1,0,0,r);
    if (i < 0) {
      mpz_t e;
      mpz_init_set_ui(e,1);
      mpz_mul_2exp(e,e,8*sizeof(long long));
      mpz_sub(x,x,e);
      mpz_clear(e);
    }
  }

  bool bit(bigint);
  mpz_ptr unsafe_get_mpz_t(void);

  friend bigint& operator++(bigint &);
  friend bigint& operator--(bigint &);
  friend bigint operator++(bigint &,int);
  friend bigint operator--(bigint &,int);

  friend bool operator==(const bigint &,const bigint &);
  friend bool operator==(const bigint &,int);
  friend bool operator==(const bigint &,unsigned int);
  friend bool operator==(const bigint &,long);
  friend bool operator==(const bigint &,unsigned long);
  friend bool operator==(const bigint &,long long);
  friend bool operator==(const bigint &,unsigned long long);
  friend bool operator==(int,const bigint &);
  friend bool operator==(unsigned int,const bigint &);
  friend bool operator==(long,const bigint &);
  friend bool operator==(unsigned long,const bigint &);
  friend bool operator==(long long,const bigint &);
  friend bool operator==(unsigned long long,const bigint &);
  
  friend bool operator!=(const bigint &,const bigint &);
  friend bool operator!=(const bigint &,int);
  friend bool operator!=(const bigint &,unsigned int);
  friend bool operator!=(const bigint &,long);
  friend bool operator!=(const bigint &,unsigned long);
  friend bool operator!=(const bigint &,long long);
  friend bool operator!=(const bigint &,unsigned long long);
  friend bool operator!=(int,const bigint &);
  friend bool operator!=(unsigned int,const bigint &);
  friend bool operator!=(long,const bigint &);
  friend bool operator!=(unsigned long,const bigint &);
  friend bool operator!=(long long,const bigint &);
  friend bool operator!=(unsigned long long,const bigint &);
  
  friend bool operator<(const bigint &,const bigint &);
  friend bool operator<(const bigint &,int);
  friend bool operator<(const bigint &,unsigned int);
  friend bool operator<(const bigint &,long);
  friend bool operator<(const bigint &,unsigned long);
  friend bool operator<(const bigint &,long long);
  friend bool operator<(const bigint &,unsigned long long);
  friend bool operator<(int,const bigint &);
  friend bool operator<(unsigned int,const bigint &);
  friend bool operator<(long,const bigint &);
  friend bool operator<(unsigned long,const bigint &);
  friend bool operator<(long long,const bigint &);
  friend bool operator<(unsigned long long,const bigint &);
  
  friend bool operator<=(const bigint &,const bigint &);
  friend bool operator<=(const bigint &,int);
  friend bool operator<=(const bigint &,unsigned int);
  friend bool operator<=(const bigint &,long);
  friend bool operator<=(const bigint &,unsigned long);
  friend bool operator<=(const bigint &,long long);
  friend bool operator<=(const bigint &,unsigned long long);
  friend bool operator<=(int,const bigint &);
  friend bool operator<=(unsigned int,const bigint &);
  friend bool operator<=(long,const bigint &);
  friend bool operator<=(unsigned long,const bigint &);
  friend bool operator<=(long long,const bigint &);
  friend bool operator<=(unsigned long long,const bigint &);

  friend bool operator>(const bigint &,const bigint &);
  friend bool operator>(const bigint &,int);
  friend bool operator>(const bigint &,unsigned int);
  friend bool operator>(const bigint &,long);
  friend bool operator>(const bigint &,unsigned long);
  friend bool operator>(const bigint &,long long);
  friend bool operator>(const bigint &,unsigned long long);
  friend bool operator>(int,const bigint &);
  friend bool operator>(unsigned int,const bigint &);
  friend bool operator>(long,const bigint &);
  friend bool operator>(unsigned long,const bigint &);
  friend bool operator>(long long,const bigint &);
  friend bool operator>(unsigned long long,const bigint &);
  
  friend bool operator>=(const bigint &,const bigint &);
  friend bool operator>=(const bigint &,int);
  friend bool operator>=(const bigint &,unsigned int);
  friend bool operator>=(const bigint &,long);
  friend bool operator>=(const bigint &,unsigned long);
  friend bool operator>=(const bigint &,long long);
  friend bool operator>=(const bigint &,unsigned long long);
  friend bool operator>=(int,const bigint &);
  friend bool operator>=(unsigned int,const bigint &);
  friend bool operator>=(long,const bigint &);
  friend bool operator>=(unsigned long,const bigint &);
  friend bool operator>=(long long,const bigint &);
  friend bool operator>=(unsigned long long,const bigint &);
  
  friend bigint operator+(const bigint &,const bigint &);
  friend bigint operator+(const bigint &,int);
  friend bigint operator+(const bigint &,unsigned int);
  friend bigint operator+(const bigint &,long);
  friend bigint operator+(const bigint &,unsigned long);
  friend bigint operator+(const bigint &,long long);
  friend bigint operator+(const bigint &,unsigned long long);
  friend bigint operator+(int,const bigint &);
  friend bigint operator+(unsigned int,const bigint &);
  friend bigint operator+(long,const bigint &);
  friend bigint operator+(unsigned long,const bigint &);
  friend bigint operator+(long long,const bigint &);
  friend bigint operator+(unsigned long long,const bigint &);

  friend bigint operator*(const bigint &,const bigint &);
  friend bigint operator*(const bigint &,int);
  friend bigint operator*(const bigint &,unsigned int);
  friend bigint operator*(const bigint &,long);
  friend bigint operator*(const bigint &,unsigned long);
  friend bigint operator*(const bigint &,long long);
  friend bigint operator*(const bigint &,unsigned long long);
  friend bigint operator*(int,const bigint &);
  friend bigint operator*(unsigned int,const bigint &);
  friend bigint operator*(long,const bigint &);
  friend bigint operator*(unsigned long,const bigint &);
  friend bigint operator*(long long,const bigint &);
  friend bigint operator*(unsigned long long,const bigint &);

  friend bigint operator-(const bigint &);

  friend bigint operator-(const bigint &,const bigint &);
  friend bigint operator-(const bigint &,int);
  friend bigint operator-(const bigint &,unsigned int);
  friend bigint operator-(const bigint &,long);
  friend bigint operator-(const bigint &,unsigned long);
  friend bigint operator-(const bigint &,long long);
  friend bigint operator-(const bigint &,unsigned long long);
  friend bigint operator-(int,const bigint &);
  friend bigint operator-(unsigned int,const bigint &);
  friend bigint operator-(long,const bigint &);
  friend bigint operator-(unsigned long,const bigint &);
  friend bigint operator-(long long,const bigint &);
  friend bigint operator-(unsigned long long,const bigint &);

  friend bigint operator/(const bigint &,const bigint &);
  friend bigint operator/(const bigint &,int);
  friend bigint operator/(const bigint &,unsigned int);
  friend bigint operator/(const bigint &,long);
  friend bigint operator/(const bigint &,unsigned long);
  friend bigint operator/(const bigint &,long long);
  friend bigint operator/(const bigint &,unsigned long long);
  friend bigint operator/(int,const bigint &);
  friend bigint operator/(unsigned int,const bigint &);
  friend bigint operator/(long,const bigint &);
  friend bigint operator/(unsigned long,const bigint &);
  friend bigint operator/(long long,const bigint &);
  friend bigint operator/(unsigned long long,const bigint &);

  friend bigint operator%(const bigint &,const bigint &);
  friend bigint operator%(const bigint &,int);
  friend bigint operator%(const bigint &,unsigned int);
  friend bigint operator%(const bigint &,long);
  friend bigint operator%(const bigint &,unsigned long);
  friend bigint operator%(const bigint &,long long);
  friend bigint operator%(const bigint &,unsigned long long);
  friend bigint operator%(int,const bigint &);
  friend bigint operator%(unsigned int,const bigint &);
  friend bigint operator%(long,const bigint &);
  friend bigint operator%(unsigned long,const bigint &);
  friend bigint operator%(long long,const bigint &);
  friend bigint operator%(unsigned long long,const bigint &);

  friend bigint operator<<(const bigint &,const bigint &);
  friend bigint operator<<(const bigint &,int);
  friend bigint operator<<(const bigint &,unsigned int);
  friend bigint operator<<(const bigint &,long);
  friend bigint operator<<(const bigint &,unsigned long);
  friend bigint operator<<(const bigint &,long long);
  friend bigint operator<<(const bigint &,unsigned long long);
  friend bigint operator<<(int,const bigint &);
  friend bigint operator<<(unsigned int,const bigint &);
  friend bigint operator<<(long,const bigint &);
  friend bigint operator<<(unsigned long,const bigint &);
  friend bigint operator<<(long long,const bigint &);
  friend bigint operator<<(unsigned long long,const bigint &);

  friend bigint operator>>(const bigint &,const bigint &);
  friend bigint operator>>(const bigint &,int);
  friend bigint operator>>(const bigint &,unsigned int);
  friend bigint operator>>(const bigint &,long);
  friend bigint operator>>(const bigint &,unsigned long);
  friend bigint operator>>(const bigint &,long long);
  friend bigint operator>>(const bigint &,unsigned long long);
  friend bigint operator>>(int,const bigint &);
  friend bigint operator>>(unsigned int,const bigint &);
  friend bigint operator>>(long,const bigint &);
  friend bigint operator>>(unsigned long,const bigint &);
  friend bigint operator>>(long long,const bigint &);
  friend bigint operator>>(unsigned long long,const bigint &);

  friend bigint& operator+=(bigint &,const bigint &);
  friend bigint& operator+=(bigint &,int);
  friend bigint& operator-=(bigint &,const bigint &);
  friend bigint& operator*=(bigint &,const bigint &);
  friend bigint& operator/=(bigint &,const bigint &);
  friend bigint& operator%=(bigint &,const bigint &);
  friend bigint& operator<<=(bigint &,const bigint &);
  friend bigint& operator>>=(bigint &,const bigint &);

  friend bigint operator&(const bigint &,const bigint &);
  friend bigint operator&(const bigint &,int);

  std::string get_str(void) const;
  friend std::ostream &operator<<(std::ostream &,const bigint &);

  operator int() const;
  operator unsigned int() const;
  operator long() const;
  operator unsigned long() const;
  operator long long() const;
  operator unsigned long long() const;
  operator bool() const;
} ;

bigint binomial(bigint,bigint);
bigint nbits(bigint);
bigint nbits0(bigint);

#endif
