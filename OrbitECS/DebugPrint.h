// DebugPrint.h
#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#include "BodyLoader.h"
#include "StateSnapshot.h"

namespace DebugPrint {
    void printLoadedBodies(const LoadedBodies& loaded);
    void exportToCsv(const LoadedBodies& loaded, const std::string& path);
    void exportStepToCsv(const HeavyBodies& heavy, const LightBodies& light,
                          const StateSnapshot& snapshot, int stepIndex,
                          const std::string& path);
}

#endif