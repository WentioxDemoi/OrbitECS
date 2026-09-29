#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <cstddef>
#include <vector>

struct Dynamic {
  std::vector<double> x, y, z;

  explicit Dynamic(int count) : x(count), y(count), z(count) {}
};

#endif