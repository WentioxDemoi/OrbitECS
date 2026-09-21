#ifndef HEAVYBODIES_H
#define HEAVYBODIES_H

#include <cstddef>
#include <string>

#include "Dynamic.h"

struct HeavyBodies {

  explicit HeavyBodies(std::size_t count)
      : mass(count), vx(count), vy(count), vz(count), ax(count), ay(count), az(count), name(count),
        dynamic_(count) {}

        std::vector<double> mass, vx, vy, vz, ax, ay, az;
        std::vector<std::string> name;
        Dynamic dynamic_;
        
};

#endif