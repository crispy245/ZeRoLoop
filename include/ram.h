#pragma once

#ifndef ram_h
#define ram_h

#include "bit.h"
#include "bigint.h"
#include <vector>

using namespace std;

const std::vector<bit> ram_read(
  const std::vector<std::vector<bit>> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint);

const std::vector<bit> ram_read(
  const std::vector<std::vector<bit>> &,
  bigint,
  bigint,
  const std::vector<bit> &);

const vector<bit> ram_read(
  const vector<std::vector<bit>> &,
  const std::vector<bit> &);

const bit ram_read(
  const std::vector<std::vector<bit>> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint,
  bigint);

const bit ram_read(
  const std::vector<bit> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint);

const bit ram_read(
  const std::vector<bit> &,
  const std::vector<bit> &);

void ram_write(
  std::vector<std::vector<bit>> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint,
  const std::vector<bit> &,
  bit = bit(0),
  bool = 1);

void ram_write(
  vector<std::vector<bit>> &,
  bigint,
  bigint,
  const vector<bit> &,
  const vector<bit> &);

void ram_write(
  vector<std::vector<bit>> &,
  const vector<bit> &,
  const vector<bit> &);

const vector<bit> ram_read_write(
  std::vector<std::vector<bit>> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint,
  std::vector<bit> &,
  bit = bit(0),
  bool = 1);

const vector<bit> ram_read_write(
  vector<std::vector<bit>> &,
  bigint,
  bigint,
  const vector<bit> &,
  vector<bit> &);

const vector<bit> ram_read_write(
  vector<std::vector<bit>> &,
  const vector<bit> &,
  vector<bit> &);

void ram_write(
  std::vector<bit> &,
  bigint,
  bigint,
  const std::vector<bit> &,
  bigint,
  const bit,
  bit = bit(0),
  bool = 1);

void ram_write(
  vector<bit> &,
  bigint,
  bigint,
  const vector<bit> &,
  const bit);

void ram_write(
  vector<bit> &,
  const vector<bit> &,
  const bit);

#endif

