#ifndef ACCELBUFFER_H
#define ACCELBUFFER_H

#include <cstddef>
#include <vector>

struct AccelBuffer {
  std::vector<double> ax, ay, az;
  explicit AccelBuffer(std::size_t n) : ax(n), ay(n), az(n) {}
};

#endif