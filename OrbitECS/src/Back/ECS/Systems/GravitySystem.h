#ifndef GRAVITYSYSTEM_H
#define GRAVITYSYSTEM_H

#include "HeavyBodies.h"
#include "LightBodies.h"
#include "StateSnapshot.h"

namespace GravitySystem {
void computeAccelerations(HeavyBodies &heavy, LightBodies &light,
                          const StateSnapshot &in);
}

#endif