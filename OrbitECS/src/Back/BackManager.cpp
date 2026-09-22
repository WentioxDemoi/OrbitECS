// BackManager.cpp
#include "BackManager.h"
#include "DebugPrint.h"
#include "Systems/GravitySystem.h"
#include "Systems/IntegrationSystem.h"

#include <iostream>
#include <string>
#include <thread>

BackManager::BackManager(HeavyBodies heavy, LightBodies light,
                         BufferExchange &buf, double dt, double simSpeedFactor)
    : heavy_(std::move(heavy)), light_(std::move(light)), buf_(buf), dt_(dt),
      simSpeedFactor_(simSpeedFactor), prevHeavyAccel_(heavy_.count_),
      prevLightAccel_(light_.count_),
      local_{StateSnapshot(heavy_.dynamic_, light_.dynamic_),
             StateSnapshot(heavy_.dynamic_, light_.dynamic_)} {
  primeAccelerations();
}

void BackManager::primeAccelerations() {
  GravitySystem::computeAccelerations(heavy_, light_, buf_.lastPublished());
}

void BackManager::run() {
  running_ = true;

  const auto period = std::chrono::seconds(1);
  auto nextTick = clock_type::now() + period;

  const int stepsPerBatch = simSpeedFactor_ / dt_;

  // int i = 0; // Debug

  while (running_) {
    const auto batchStart = clock_type::now();
    // Amorce le batch depuis le dernier état publié
    local_[0] = buf_.lastPublished();
    localCurr_ = 0;

    processBatch(stepsPerBatch);

    // Un seul transfert vers BufferExchange, une seule publication par batch
    StateSnapshot &finalState = local_[localCurr_];
    StateSnapshot &slot = buf_.writeSlot();
    slot.simTime = finalState.simTime;
    slot.heavyDynamic = finalState.heavyDynamic;
    slot.lightDynamic = finalState.lightDynamic;
    buf_.publish();

    const auto batchEnd = clock_type::now(); // fin du chrono
    const auto batchDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(batchEnd -
                                                              batchStart);
    std::cerr << "[BackManager] Batch de " << stepsPerBatch
              << " pas calculé en " << batchDuration.count() << " ms\n";

    // if (i == 0 || i == 10) {
    //     DebugPrint::exportStepToCsv(heavy_, light_, slot, i,
    //     "../../Debug/debug_steps" + std::to_string(i) + ".csv");
    // } else if (i > 10)
    //     exit(0);
    // i++;

    // Attend le prochain top, si le batch a dépassé la période,
    // saute directement au prochain top plutôt que d'accumuler du retard.
    const auto now = clock_type::now();
    if (now < nextTick) {
      std::this_thread::sleep_until(nextTick);
      nextTick += period;
    } else {
      const auto overrun = now - nextTick;
      const auto missedTicks = overrun / period + 1;
      if (missedTicks > 1) {
        const auto overrunMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(overrun)
                .count();
        std::cerr << "[BackManager] Batch trop lent : dépassement de "
                  << overrunMs << " ms, " << (missedTicks - 1)
                  << " tick(s) sauté(s)\n";
      }
      nextTick += period * missedTicks;
    }
  }
}

void BackManager::stop() { running_ = false; }

void BackManager::processBatch(int stepsPerBatch) {
  for (int s = 0; s < stepsPerBatch && running_; ++s) {
    // Snapshot tmp pour povoir calculer toutes les steps entre 2 batchs
    StateSnapshot &in = local_[localCurr_];
    StateSnapshot &out = local_[1 - localCurr_];

    IntegrationSystem::updatePositions(heavy_, light_, in, out, dt_);

    prevHeavyAccel_.ax = heavy_.ax;
    prevHeavyAccel_.ay = heavy_.ay;
    prevHeavyAccel_.az = heavy_.az;
    prevLightAccel_.ax = light_.ax;
    prevLightAccel_.ay = light_.ay;
    prevLightAccel_.az = light_.az;

    GravitySystem::computeAccelerations(heavy_, light_, out);

    IntegrationSystem::updateVelocities(heavy_, light_, prevHeavyAccel_,
                                        prevLightAccel_, dt_);

    localCurr_ = 1 - localCurr_;
  }
}