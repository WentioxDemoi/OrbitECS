#ifndef STATESNAPSHOT_H
#define STATESNAPSHOT_H

#include "Dynamic.h"

struct StateSnapshot {
    double simTime = 0.0;
    Dynamic &heavyDynamic;
    Dynamic &lightDynamic;

    StateSnapshot(Dynamic &heavy, Dynamic &light)
        : heavyDynamic(heavy), lightDynamic(light) {}

    void swap(StateSnapshot& other) noexcept {
        std::swap(simTime, other.simTime);
        heavyDynamic.swap(other.heavyDynamic);
        lightDynamic.swap(other.lightDynamic);
    }
};

#endif