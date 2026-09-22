#include "GravitySystem.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
constexpr double kSoftening2 = 1e-6; // km carré, softening de 1 m
}

namespace GravitySystem {

// in doit être last publish
void computeAccelerations(HeavyBodies &heavy, LightBodies &light,
                          const StateSnapshot &in) {
  const std::size_t nh = heavy.count_;
  const std::size_t nl = light.count_;

  std::fill(heavy.ax.begin(), heavy.ax.end(), 0.0);
  std::fill(heavy.ay.begin(), heavy.ay.end(), 0.0);
  std::fill(heavy.az.begin(), heavy.az.end(), 0.0);

  const auto &hx = in.heavyDynamic.x;
  const auto &hy = in.heavyDynamic.y;
  const auto &hz = in.heavyDynamic.z;

  // Heavy <-> Heavy

  for (std::size_t i = 0; i < nh; ++i) {
    for (std::size_t j = i + 1; j < nh; ++j) {

      const double dx = hx[j] - hx[i];
      const double dy = hy[j] - hy[i];
      const double dz = hz[j] - hz[i];

      const double dist2 = dx * dx + dy * dy + dz * dz + kSoftening2;

      const double invDist3 = 1.0 / (dist2 * std::sqrt(dist2));

      const double fi = heavy.gm[j] * invDist3;

      const double fj = heavy.gm[i] * invDist3;

      heavy.ax[i] += fi * dx;
      heavy.ay[i] += fi * dy;
      heavy.az[i] += fi * dz;

      heavy.ax[j] -= fj * dx;
      heavy.ay[j] -= fj * dy;
      heavy.az[j] -= fj * dz;
    }
  }

  // Heavy -> Light

  const auto &lx = in.lightDynamic.x;
  const auto &ly = in.lightDynamic.y;
  const auto &lz = in.lightDynamic.z;

  for (std::size_t k = 0; k < nl; ++k) {

    double ax = 0.0;
    double ay = 0.0;
    double az = 0.0;

    for (std::size_t i = 0; i < nh; ++i) {

      const double dx = hx[i] - lx[k];
      const double dy = hy[i] - ly[k];
      const double dz = hz[i] - lz[k];

      const double dist2 = dx * dx + dy * dy + dz * dz + kSoftening2;

      const double invDist3 = 1.0 / (dist2 * std::sqrt(dist2));

      const double f = heavy.gm[i] * invDist3;

      ax += f * dx;
      ay += f * dy;
      az += f * dz;
    }

    light.ax[k] = ax;
    light.ay[k] = ay;
    light.az[k] = az;
  }
}

} // namespace GravitySystem