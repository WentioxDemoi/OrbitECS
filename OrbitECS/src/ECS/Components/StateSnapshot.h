#include "HeavyBodies.h"
#include "LightBodies.h"

struct StateSnapshot {
    double simTime = 0.0;
    HeavyBodies::Dynamic heavyDynamic;
    LightBodies::Dynamic lightDynamic;

    StateSnapshot(std::size_t heavyCount, std::size_t lightCount)
        : heavyDynamic(heavyCount), lightDynamic(lightCount) {}

    void swap(StateSnapshot& other) noexcept {
        std::swap(simTime, other.simTime);
        heavyDynamic.swap(other.heavyDynamic);
        lightDynamic.swap(other.lightDynamic);
    }
};