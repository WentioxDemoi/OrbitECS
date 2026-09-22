#ifndef INTEGRATIONSYSTEM_H
#define INTEGRATIONSYSTEM_H

#include "HeavyBodies.h"
#include "LightBodies.h"
#include "StateSnapshot.h"
#include "AccelBuffer.h"

namespace IntegrationSystem {
        // Utilise v(t) et a(t) (déjà dans heavy/light), écrit les nouvelles positions dans out
    void updatePositions(HeavyBodies& heavy, LightBodies& light,
                          const StateSnapshot& in, StateSnapshot& out, double dt);

    // Combine prevAccel (a(t)) et heavy/light.ax actuel (a(t+dt)) pour mettre à jour v
    void updateVelocities(HeavyBodies& heavy, LightBodies& light,
                           const AccelBuffer& prevHeavyAccel, const AccelBuffer& prevLightAccel,
                           double dt);
}

#endif