// BackManager.h
#ifndef BACKMANAGER_H
#define BACKMANAGER_H

#include "HeavyBodies.h"
#include "LightBodies.h"
#include "AccelBuffer.h"
#include "BufferExchange.h"
#include <atomic>
#include <chrono>

using clock_type = std::chrono::steady_clock;

class BackManager {
public:
    BackManager(HeavyBodies heavy, LightBodies light, BufferExchange& buf, double dt);

    void run();      // boucle bloquante — à lancer dans son propre thread
    void stop();      // thread-safe, appelable depuis un autre thread

private:
    void primeAccelerations();   // amorce a(0) avant la première itération

    HeavyBodies heavy_;
    LightBodies light_;
    BufferExchange& buf_;

    // dt fixe pour le moment, sera amené à être dynamique
    double dt_;

    AccelBuffer prevHeavyAccel_;
    AccelBuffer prevLightAccel_;

    std::atomic<bool> running_{false};
};

#endif