#include "IntegrationSystem.h"
#include <cstddef>

namespace IntegrationSystem {

void updatePositions(HeavyBodies &heavy, LightBodies &light,
                     const StateSnapshot &in, StateSnapshot &out, double dt) {
  const std::size_t nh = heavy.count_;
  const std::size_t nl = light.count_;
  const double half_dt2 = 0.5 * dt * dt;

  out.simTime = in.simTime + dt;

  for (std::size_t i = 0; i < nh; ++i) {
    out.heavyDynamic.x[i] =
        in.heavyDynamic.x[i] + heavy.vx[i] * dt + heavy.ax[i] * half_dt2;
    out.heavyDynamic.y[i] =
        in.heavyDynamic.y[i] + heavy.vy[i] * dt + heavy.ay[i] * half_dt2;
    out.heavyDynamic.z[i] =
        in.heavyDynamic.z[i] + heavy.vz[i] * dt + heavy.az[i] * half_dt2;
  }

  for (std::size_t k = 0; k < nl; ++k) {
    out.lightDynamic.x[k] =
        in.lightDynamic.x[k] + light.vx[k] * dt + light.ax[k] * half_dt2;
    out.lightDynamic.y[k] =
        in.lightDynamic.y[k] + light.vy[k] * dt + light.ay[k] * half_dt2;
    out.lightDynamic.z[k] =
        in.lightDynamic.z[k] + light.vz[k] * dt + light.az[k] * half_dt2;
  }
}

void updateVelocities(HeavyBodies &heavy, LightBodies &light,
                      const AccelBuffer &prevHeavyAccel,
                      const AccelBuffer &prevLightAccel, double dt) {
  const std::size_t nh = heavy.mass.size();
  const std::size_t nl = light.ax.size();
  const double half_dt = 0.5 * dt;

  for (std::size_t i = 0; i < nh; ++i) {
    heavy.vx[i] += (prevHeavyAccel.ax[i] + heavy.ax[i]) * half_dt;
    heavy.vy[i] += (prevHeavyAccel.ay[i] + heavy.ay[i]) * half_dt;
    heavy.vz[i] += (prevHeavyAccel.az[i] + heavy.az[i]) * half_dt;
  }

  for (std::size_t k = 0; k < nl; ++k) {
    light.vx[k] += (prevLightAccel.ax[k] + light.ax[k]) * half_dt;
    light.vy[k] += (prevLightAccel.ay[k] + light.ay[k]) * half_dt;
    light.vz[k] += (prevLightAccel.az[k] + light.az[k]) * half_dt;
  }
}

} // namespace IntegrationSystem