#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <cstddef>
#include <vector>

struct Dynamic {
  std::vector<double> x, y, z;

  explicit Dynamic(std::size_t count) : x(count), y(count), z(count) {}

  void swap(Dynamic &other) noexcept {
    x.swap(other.x);
    y.swap(other.y);
    z.swap(other.z);
  }
};

#endif