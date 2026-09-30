#ifndef STATESNAPSHOT_H
#define STATESNAPSHOT_H

#include "Dynamic.h"
#include <chrono>

struct StateSnapshot {
  double simTime = 0.0;
  std::chrono::system_clock::time_point epoch;
  Dynamic heavyDynamic;
  Dynamic lightDynamic;

  StateSnapshot(Dynamic heavy, Dynamic light, std::chrono::system_clock::time_point epoch)
      : heavyDynamic(heavy), lightDynamic(light), epoch(epoch) {}
};

#endif