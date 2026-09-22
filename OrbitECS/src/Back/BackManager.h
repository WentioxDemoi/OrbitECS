// BackManager.h
#ifndef BACKMANAGER_H
#define BACKMANAGER_H

#include "HeavyBodies.h"
#include "LightBodies.h"
#include "BufferExchange.h"
#include "AccelBuffer.h"
#include <atomic>
#include <chrono>

class BackManager {
public:
    // dt : pas Verlet fixe, en secondes simulées (précision de l'intégration)
    // simSpeedFactor : secondes simulées par seconde réelle (vitesse de la simu)
    BackManager(HeavyBodies heavy, LightBodies light, BufferExchange& buf,
                double dt, double simSpeedFactor);

    void run();
    void stop();

private:
    using clock_type = std::chrono::steady_clock;

    void primeAccelerations();

    HeavyBodies heavy_;
    LightBodies light_;
    BufferExchange& buf_;

    int dt_;
    int simSpeedFactor_;

    AccelBuffer prevHeavyAccel_;
    AccelBuffer prevLightAccel_;

    StateSnapshot local_[2];
    int localCurr_ = 0;

    std::atomic<bool> running_{false};
};

#endif