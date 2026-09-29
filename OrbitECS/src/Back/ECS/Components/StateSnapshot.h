#ifndef STATESNAPSHOT_H
#define STATESNAPSHOT_H

#include "Dynamic.h"

struct StateSnapshot {
  int simTime = 0.0;
  Dynamic heavyDynamic;
  Dynamic lightDynamic;

  StateSnapshot(Dynamic heavy, Dynamic light)
      : heavyDynamic(heavy), lightDynamic(light) {}
};

#endif