#include "BackManager.h"
#include "Systems/GravitySystem.h"
#include "Systems/IntegrationSystem.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <thread>

BackManager::BackManager(HeavyBodies heavy, LightBodies light,
                         BufferExchange &buf, std::chrono::system_clock::time_point epoch, QObject *parent)
    : heavy_(std::move(heavy)), light_(std::move(light)), buf_(buf), dt_(1),
      simSpeedFactor_(1), prevHeavyAccel_(heavy_.count_),
      prevLightAccel_(light_.count_),
      local_{StateSnapshot(heavy_.dynamic_, light_.dynamic_, epoch),
             StateSnapshot(heavy_.dynamic_, light_.dynamic_, epoch)},
      QObject(parent) {
  primeAccelerations();
}

void BackManager::primeAccelerations() {
  GravitySystem::computeAccelerations(heavy_, light_, buf_.lastPublished());
}

void BackManager::run() {
  running_ = true;

  constexpr auto period = std::chrono::seconds(1);
  auto nextTick = clock_type::now() + period;

  while (running_) {
    const std::int64_t simSpeedFactor =
      simSpeedFactor_.load(std::memory_order_relaxed);
    const int maxStepSeconds = dt_.load(std::memory_order_relaxed);
    const std::uint64_t absoluteDuration = simSpeedFactor < 0
      ? static_cast<std::uint64_t>(-(simSpeedFactor + 1)) + 1
      : static_cast<std::uint64_t>(simSpeedFactor);
    const std::uint64_t stepsPerBatch =
      absoluteDuration / maxStepSeconds +
      (absoluteDuration % maxStepSeconds != 0 ? 1 : 0);

    const auto batchStart = clock_type::now();

    local_[0] = buf_.lastPublished();
    localCurr_ = 0;

    processBatch(simSpeedFactor, maxStepSeconds);

    StateSnapshot &finalState = local_[localCurr_];

    StateSnapshot &slot = buf_.writeSlot();

    slot.simTime = finalState.simTime;
    slot.heavyDynamic = finalState.heavyDynamic;
    slot.lightDynamic = finalState.lightDynamic;

    buf_.publish();

    const auto batchEnd = clock_type::now();

    const auto batchDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(batchEnd -
                                                              batchStart);

    std::cerr << "[BackManager] Batch de " << stepsPerBatch
              << " pas calculé en " << batchDuration.count() << " ms\n";

    const auto now = clock_type::now();

    if (now < nextTick) {
      std::this_thread::sleep_until(nextTick);
      nextTick += period;
    } else {
      const auto overrun = now - nextTick;
      const auto missedTicks = overrun / period + 1;

      if (missedTicks > 1) {
        emit simulationOverrun(maxStepSeconds + 1);
      }

      nextTick += period * missedTicks;
    }
  }
}

void BackManager::stop() { running_ = false; }

void BackManager::processBatch(std::int64_t simSpeedFactor,
                              int maxStepSeconds) {
  if (maxStepSeconds <= 0) {
    return;
  }

  const int direction = simSpeedFactor < 0 ? -1 : 1;

  std::int64_t remainingSeconds = simSpeedFactor;

  while (remainingSeconds != 0 && running_) {
    const std::uint64_t remainingMagnitude = remainingSeconds < 0
        ? static_cast<std::uint64_t>(-(remainingSeconds + 1)) + 1
        : static_cast<std::uint64_t>(remainingSeconds);

    const std::int64_t stepMagnitude = std::min<std::uint64_t>(
        remainingMagnitude, static_cast<std::uint64_t>(maxStepSeconds));

    const std::int64_t stepSeconds = direction * stepMagnitude;

    StateSnapshot &in = local_[localCurr_];
    StateSnapshot &out = local_[1 - localCurr_];

    IntegrationSystem::updatePositions(heavy_, light_, in, out, stepSeconds);

    prevHeavyAccel_.ax = heavy_.ax;
    prevHeavyAccel_.ay = heavy_.ay;
    prevHeavyAccel_.az = heavy_.az;

    prevLightAccel_.ax = light_.ax;
    prevLightAccel_.ay = light_.ay;
    prevLightAccel_.az = light_.az;

    GravitySystem::computeAccelerations(heavy_, light_, out);

    IntegrationSystem::updateVelocities(heavy_, light_, prevHeavyAccel_,
                                        prevLightAccel_, stepSeconds);

    localCurr_ = 1 - localCurr_;

    remainingSeconds -= stepSeconds;
  }
}