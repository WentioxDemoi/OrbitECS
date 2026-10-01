#include "FrontManager.h"

#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QTimeZone>
#include <QVector4D>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace {

constexpr f64 kPositionScale = 1e-6;

constexpr f32 kSphereRadius = 1.5F;
constexpr f32 kSunVisualRadius = 0.6F;
constexpr f32 kPlanetVisualBase = 0.05F;
constexpr f32 kEarthRadiusKm = 6371.0F;
constexpr f32 kMinVisualRadius = 0.015F;

constexpr std::int32_t kTickIntervalMs = 16;

} // namespace

FrontManager::FrontManager(BufferExchange &exchange,
                           std::vector<BodyMetaData> metaData, QObject *parent)
    : QObject(parent), exchange_(exchange), metaData_(std::move(metaData)) {

  const StateSnapshot &initial = exchange_.target();

  heavyCount_ = static_cast<std::uint32_t>(initial.heavyDynamic.x.size());
  lightCount_ = static_cast<std::uint32_t>(initial.lightDynamic.x.size());

  std::vector<f32> heavyScale(heavyCount_);
  std::vector<QVector4D> heavyColor(heavyCount_);

  for (std::uint32_t i = 0; i < heavyCount_; ++i) {
    f32 visualRadius = kSunVisualRadius;

    if (i != 0) {
      visualRadius = kPlanetVisualBase *
                     std::cbrt(static_cast<f32>(metaData_[i].radius) /
                               kEarthRadiusKm);
    }

    visualRadius = std::max(visualRadius, kMinVisualRadius);

    heavyScale[i] = visualRadius / kSphereRadius;

    const f64 hue =
        heavyCount_ > 1 ? static_cast<f64>(i) / static_cast<f64>(heavyCount_)
                        : 0.0;

    const QColor c = QColor::fromHsvF(static_cast<f32>(hue), 0.55F, 1.0F);

    heavyColor[i] =
        QVector4D(static_cast<f32>(c.redF()), static_cast<f32>(c.greenF()),
                  static_cast<f32>(c.blueF()), 1.0F);
  }

  const f32 lightVisualScale = heavyCount_ > 0 ? heavyScale[0] / 4.0F : 1.0F;

  const std::vector<f32> lightScale(lightCount_, lightVisualScale);
  const std::vector<QVector4D> lightColor(lightCount_,
                                          QVector4D(1.0F, 1.0F, 1.0F, 1.0F));

  heavyX_.resize(heavyCount_);
  heavyY_.resize(heavyCount_);
  heavyZ_.resize(heavyCount_);

  lightX_.resize(lightCount_);
  lightY_.resize(lightCount_);
  lightZ_.resize(lightCount_);

  interpolate(initial.heavyDynamic, initial.heavyDynamic, 0.0, heavyX_, heavyY_,
              heavyZ_);
  interpolate(initial.lightDynamic, initial.lightDynamic, 0.0, lightX_, lightY_,
              lightZ_);

  heavyInstancing_.setBodies(heavyX_, heavyY_, heavyZ_, heavyScale, heavyColor);
  lightInstancing_.setBodies(lightX_, lightY_, lightZ_, lightScale, lightColor);

  timer_.setInterval(kTickIntervalMs);
  connect(&timer_, &QTimer::timeout, this, &FrontManager::tick);
}

void FrontManager::start() {
  clock_.start();
  timer_.start();
}

void FrontManager::setDt(qint32 dt) {
  const auto clamped = static_cast<std::uint16_t>(
      std::clamp<std::int32_t>(dt, 0, std::numeric_limits<std::uint16_t>::max()));

  if (clamped == dt_) {
    return;
  }

  dt_ = clamped;
  emit dtChanged(static_cast<qint32>(dt_));
}

void FrontManager::setSimSpeedFactor(qint32 simSpeedFactor) {
  if (simSpeedFactor == simSpeedFactor_) {
    return;
  }

  simSpeedFactor_ = simSpeedFactor;
  emit simSpeedFactorChanged(simSpeedFactor_);
}

void FrontManager::tick() {
  const bool advanced = exchange_.tryAdvance();

  const std::int64_t now = static_cast<std::int64_t>(clock_.elapsed());

  if (advanced) {
    if (lastAdvanceMs_ >= 0) {
      advanceIntervalMs_ = now - lastAdvanceMs_;
    }
    lastAdvanceMs_ = now;
  }

  f64 alpha = 1.0;

  if (advanceIntervalMs_ > 0 && lastAdvanceMs_ >= 0) {
    alpha = static_cast<f64>(now - lastAdvanceMs_) /
            static_cast<f64>(advanceIntervalMs_);
    alpha = std::clamp(alpha, 0.0, 1.0);
  }

  const StateSnapshot &prev = exchange_.prev();
  const StateSnapshot &target = exchange_.target();

  interpolate(prev.heavyDynamic, target.heavyDynamic, alpha, heavyX_, heavyY_,
              heavyZ_);
  interpolate(prev.lightDynamic, target.lightDynamic, alpha, lightX_, lightY_,
              lightZ_);

  updateTimeStamp(prev, target, alpha);

  heavyInstancing_.updatePositions(heavyX_, heavyY_, heavyZ_);
  lightInstancing_.updatePositions(lightX_, lightY_, lightZ_);
}

void FrontManager::interpolate(const Dynamic &prev, const Dynamic &target,
                               f64 alpha, std::vector<f32> &outX,
                               std::vector<f32> &outY,
                               std::vector<f32> &outZ) const {

  const auto count = static_cast<std::uint32_t>(target.x.size());

  for (std::uint32_t i = 0; i < count; ++i) {
    const f64 x = prev.x[i] + (target.x[i] - prev.x[i]) * alpha;
    const f64 y = prev.y[i] + (target.y[i] - prev.y[i]) * alpha;
    const f64 z = prev.z[i] + (target.z[i] - prev.z[i]) * alpha;

    outX[i] = static_cast<f32>(x * kPositionScale);
    outY[i] = static_cast<f32>(y * kPositionScale);
    outZ[i] = static_cast<f32>(z * kPositionScale);
  }
}

void FrontManager::updateTimeStamp(const StateSnapshot &prev,
                                   const StateSnapshot &target, f64 alpha) {

  const f64 simTime = prev.simTime + (target.simTime - prev.simTime) * alpha;

  const std::int64_t epochSecs =
      std::chrono::duration_cast<std::chrono::seconds>(
          target.epoch.time_since_epoch())
          .count();

  const std::int64_t totalSecs =
      epochSecs + static_cast<std::int64_t>(std::floor(simTime));

  const QString date =
      QDateTime::fromSecsSinceEpoch(totalSecs, QTimeZone::UTC)
          .toString("yyyy-MM-dd HH:mm:ss");

  if (date != dateString_) {
    dateString_ = date;
    emit dateStringChanged();
  }
}

QString FrontManager::heavyName(qint32 index) const {
  if (index < 0 || static_cast<std::uint32_t>(index) >= metaData_.size()) {
    return QString();
  }

  return QString::fromStdString(metaData_[static_cast<std::uint32_t>(index)].name);
}

QVector3D FrontManager::heavyPosition(qint32 index) const {
  if (index < 0 || static_cast<std::uint32_t>(index) >= heavyCount_) {
    return QVector3D();
  }

  const auto i = static_cast<std::uint32_t>(index);
  return QVector3D(heavyX_[i], heavyY_[i], heavyZ_[i]);
}

QString FrontManager::heavyText(qint32 index) const {
  if (index < 0 || static_cast<std::uint32_t>(index) >= heavyCount_) {
    return QString();
  }

  return QString::fromStdString(metaData_[static_cast<std::uint32_t>(index)].text);
}