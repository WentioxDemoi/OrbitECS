#ifndef BACKMANAGER_H
#define BACKMANAGER_H

#include "AccelBuffer.h"
#include "BufferExchange.h"
#include "HeavyBodies.h"
#include "LightBodies.h"
#include <QtCore/qobject.h>
#include <atomic>
#include <chrono>
#include <cstdint>

class BackManager : public QObject {
  Q_OBJECT
public:
  // dt : pas Verlet fixe, en secondes simulées (précision de l'intégration)
  // simSpeedFactor : secondes simulées par seconde réelle (vitesse de la simu)
  BackManager(HeavyBodies heavy, LightBodies light, BufferExchange &buf,
              QObject *parent);

  void run();
  void processBatch(std::int64_t simSpeedFactor, int maxStepSeconds);
  void stop();

  void onSimSpeedFactorChanged(int simSpeedFactor) {
    simSpeedFactor_.store(simSpeedFactor, std::memory_order_relaxed);
  }
  void onDtChanged(int dt) {
    dt_.store(dt > 0 ? dt : 1, std::memory_order_relaxed);
  }

signals:
  void simulationOverrun(int newDt);

private:
  using clock_type = std::chrono::steady_clock;

  void primeAccelerations();

  HeavyBodies heavy_;
  LightBodies light_;
  BufferExchange &buf_;

  std::atomic<int> dt_{1};
  std::atomic<std::int64_t> simSpeedFactor_{1};

  AccelBuffer prevHeavyAccel_;
  AccelBuffer prevLightAccel_;

  StateSnapshot local_[2];
  uint8_t localCurr_ = 0;

  std::atomic<bool> running_{false};
};

#endif