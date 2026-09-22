#include "BackManager.h"
#include "Systems/GravitySystem.h"
#include "Systems/IntegrationSystem.h"

#include <iostream>
#include <thread>

BackManager::BackManager(HeavyBodies heavy, LightBodies light, BufferExchange& buf, double dt)
    : heavy_(std::move(heavy)), light_(std::move(light)), buf_(buf), dt_(dt),
      prevHeavyAccel_(heavy_.count_), prevLightAccel_(light_.count_) {
    primeAccelerations();
}

void BackManager::primeAccelerations() {
    GravitySystem::computeAccelerations(heavy_, light_, buf_.lastPublished());
}

void BackManager::run() {
    running_ = true;

    const auto period = std::chrono::seconds(10);
    auto nextTick = clock_type::now() + period;

    while (running_) {
        const StateSnapshot& in = buf_.lastPublished();
        StateSnapshot& out = buf_.writeSlot();

        IntegrationSystem::updatePositions(heavy_, light_, in, out, dt_);

        prevHeavyAccel_.ax = heavy_.ax;
        prevHeavyAccel_.ay = heavy_.ay;
        prevHeavyAccel_.az = heavy_.az;
        prevLightAccel_.ax = light_.ax;
        prevLightAccel_.ay = light_.ay;
        prevLightAccel_.az = light_.az;

        GravitySystem::computeAccelerations(heavy_, light_, out);

        IntegrationSystem::updateVelocities(heavy_, light_, prevHeavyAccel_, prevLightAccel_, dt_);

        // Publish des résultats grâce au bufferExchange
        buf_.publish();

        // Attend le prochain top toutes les 10s ; si le calcul a dépassé,
        // saute directement à la dizaine suivante plutôt que d'accumuler du retard.
        const auto now = clock_type::now();
        if (now < nextTick) {
            std::this_thread::sleep_until(nextTick);
            nextTick += period;
        } else {
            const auto overrun = now - nextTick;
            const auto missedTicks = overrun / period + 1;
            if (missedTicks > 1) {
                const auto overrunMs = std::chrono::duration_cast<std::chrono::milliseconds>(overrun).count();
                std::cerr << "[BackManager] Calcul trop lent : dépassement de "
                          << overrunMs << " ms, "
                          << (missedTicks - 1) << " tick(s) sauté(s)\n";
            }
            nextTick += period * missedTicks;
        }
    }
}

void BackManager::stop() {
    running_ = false;
}