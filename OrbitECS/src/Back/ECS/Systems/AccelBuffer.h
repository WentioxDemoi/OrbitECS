#ifndef ACCELBUFFER_H
#define ACCELBUFFER_H

#include <cstddef>
#include <vector>

struct AccelBuffer {
  std::vector<double> ax, ay, az;
  size_t count_;
  explicit AccelBuffer(size_t n) : ax(n), ay(n), az(n), count_(n) {}
};

#endif