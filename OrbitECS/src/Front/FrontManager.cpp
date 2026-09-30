#include "FrontManager.h"

#include <QColor>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <QDateTime>
#include <QDebug>
#include <QTimeZone>

namespace {
constexpr double kPositionScale = 1e-6;
constexpr double kVisualRadiusScale = 20.0;
constexpr float kSphereRadius = 1.5f;
} // namespace

FrontManager::FrontManager(BufferExchange &exchange,
                           std::vector<BodyMetaData> metaData, QObject *parent)
    : exchange_(exchange), metaData_(std::move(metaData)), QObject(parent) {

  const StateSnapshot &initial = exchange_.target();

  heavyCount_ = static_cast<uint32_t>(initial.heavyDynamic.x.size());
  lightCount_ = static_cast<uint32_t>(initial.lightDynamic.x.size());

  std::vector<float> heavyScale(heavyCount_);
  std::vector<QVector4D> heavyColor(heavyCount_);

  for (uint32_t i = 0; i < heavyCount_; ++i) {
    float visualRadius;

    if (i == 0) {
      visualRadius = 0.6f;
    } else {
      visualRadius = 0.05f * std::cbrt(float(metaData_[i].radius) / 6371.0f);
    }

    visualRadius = std::max(visualRadius, 0.015f);
    heavyScale[i] = visualRadius / kSphereRadius;

    const qreal hue = heavyCount_ > 1 ? static_cast<double>(i) /
                                            static_cast<double>(heavyCount_)
                                      : 0.0;
    const QColor c = QColor::fromHsvF(hue, 0.55, 1.0);

    heavyColor[i] =
        QVector4D(float(c.redF()), float(c.greenF()), float(c.blueF()), 1.0f);
  }

  const float lightVisualScale = heavyCount_ > 0 ? heavyScale[0] / 4.0f : 1.0f;

  const std::vector<float> lightScale(lightCount_, lightVisualScale);
  const std::vector<QVector4D> lightColor(lightCount_,
                                          QVector4D(1.0f, 1.0f, 1.0f, 1.0f));

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

  timer_.setInterval(16);

  connect(&timer_, &QTimer::timeout, this, &FrontManager::tick);
}

void FrontManager::start() {
  clock_.start();
  timer_.start();
}

void FrontManager::tick() {
  const bool advanced = exchange_.tryAdvance();

  const qint64 now = clock_.elapsed();

  if (advanced) {
    if (lastAdvanceMs_ >= 0) {
      advanceIntervalMs_ = now - lastAdvanceMs_;
    }

    lastAdvanceMs_ = now;
  }

  double alpha = 1.0;

  if (advanceIntervalMs_ > 0 && lastAdvanceMs_ >= 0) {

    alpha = static_cast<double>(now - lastAdvanceMs_) /
            static_cast<double>(advanceIntervalMs_);

    alpha = std::clamp(alpha, 0.0, 1.0);
  }

  const StateSnapshot &prev = exchange_.prev();

  const StateSnapshot &target = exchange_.target();

  

  interpolate(prev.heavyDynamic, target.heavyDynamic, alpha, heavyX_, heavyY_,
              heavyZ_);

  interpolate(prev.lightDynamic, target.lightDynamic, alpha, lightX_, lightY_,
              lightZ_);

const double simTime =
    prev.simTime + (target.simTime - prev.simTime) * alpha;

const qint64 epochSecs = std::chrono::duration_cast<std::chrono::seconds>(
                             target.epoch.time_since_epoch())
                             .count();

const qint64 totalSecs = epochSecs + static_cast<qint64>(std::floor(simTime));

const QString date = QDateTime::fromSecsSinceEpoch(totalSecs, QTimeZone::UTC)
                         .toString("yyyy-MM-dd HH:mm:ss");

if (date != dateString_) {
  dateString_ = date;
  emit dateStringChanged();
}

  heavyInstancing_.updatePositions(heavyX_, heavyY_, heavyZ_);
  lightInstancing_.updatePositions(lightX_, lightY_, lightZ_);
}

void FrontManager::interpolate(const Dynamic &prev, const Dynamic &target,
                               double alpha, std::vector<float> &outX,
                               std::vector<float> &outY,
                               std::vector<float> &outZ) const {

  const std::size_t count = target.x.size();

  for (std::size_t i = 0; i < count; ++i) {
    const double x = prev.x[i] + (target.x[i] - prev.x[i]) * alpha;

    const double y = prev.y[i] + (target.y[i] - prev.y[i]) * alpha;

    const double z = prev.z[i] + (target.z[i] - prev.z[i]) * alpha;

    outX[i] = static_cast<float>(x * kPositionScale);

    outY[i] = static_cast<float>(y * kPositionScale);

    outZ[i] = static_cast<float>(z * kPositionScale);
  }
}

QString FrontManager::heavyName(int index) const {
  if (index < 0 || static_cast<uint32_t>(index) >= metaData_.size()) {
    return QString();
  }

  return QString::fromStdString(
      metaData_[static_cast<std::size_t>(index)].name);
}

QVector3D FrontManager::heavyPosition(int index) const {
  if (index < 0 || static_cast<uint32_t>(index) >= heavyCount_) {
    return QVector3D();
  }

  const std::size_t i = static_cast<std::size_t>(index);

  return QVector3D(heavyX_[i], heavyY_[i], heavyZ_[i]);
}