#ifndef LIGHTBODIES_H
#define LIGHTBODIES_H

#include "Dynamic.h"

// LightBodies.h
struct LightBodies {
    explicit LightBodies(std::size_t count) : dynamic_(count) {}

    Dynamic dynamic_;
    // vx, vy, vz
};

#endif