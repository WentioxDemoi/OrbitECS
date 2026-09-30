#ifndef LOADEDBODIES_H
#define LOADEDBODIES_H

#include "BodyMetaData.h"
#include "HeavyBodies.h"
#include "LightBodies.h"
#include <vector>

struct LoadedBodies {
  HeavyBodies heavy;
  LightBodies light;
  std::vector<BodyMetaData> meta;
  std::chrono::system_clock::time_point epoch;
};

#endif