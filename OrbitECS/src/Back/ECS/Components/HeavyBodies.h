#ifndef HEAVYBODIES_H
#define HEAVYBODIES_H

#include <cstddef>
#include <string>
#include <vector>

#include "Dynamic.h"

struct HeavyBodies {

  explicit HeavyBodies(std::size_t count)
      : mass(count), gm(count), vx(count), vy(count), vz(count), ax(count),
        ay(count), az(count), name(count), dynamic_(count), count_(count) {}

  std::vector<double> mass;
  std::vector<double> gm; // == G * mass

  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> vz;

  std::vector<double> ax;
  std::vector<double> ay;
  std::vector<double> az;

  std::vector<std::string> name;

  Dynamic dynamic_;
  int count_;
};

#endif