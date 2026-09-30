#ifndef LIGHTBODIES_H
#define LIGHTBODIES_H

#include "Dynamic.h"

// LightBodies.h
struct LightBodies {
  explicit LightBodies(size_t count)
      : dynamic_(count), vx(count), vy(count), vz(count), ax(count), ay(count),
        az(count), count_(count) {}

  Dynamic dynamic_;
  std::vector<double> vx, vy, vz, ax, ay, az;
  size_t count_;
};

#endif