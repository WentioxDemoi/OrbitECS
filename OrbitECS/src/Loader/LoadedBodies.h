#ifndef LOADEDBODIES_H
#define LOADEDBODIES_H

#include <vector>
#include "HeavyBodies.h"
#include "LightBodies.h"
#include "BodyMetaData.h"

struct LoadedBodies {
    HeavyBodies heavy;
    LightBodies light;
    std::vector<BodyMetaData> meta;
};

#endif