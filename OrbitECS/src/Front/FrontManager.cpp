#include "FrontManager.h"

#include <QColor>
#include <algorithm>

namespace {
constexpr double kPositionScale = 1e-6;
constexpr double kVisualRadiusScale = 20.0;
constexpr float kSphereRadius = 1.5f;
} // namespace

FrontManager::FrontManager(BufferExchange &exchange,
                           std::vector<BodyMetaData> metaData, QObject *parent)
    : QObject(parent), exchange_(exchange), metaData_(std::move(metaData)) {
  const StateSnapshot &initial = exchange_.target();
  heavyCount_ = int(initial.heavyDynamic.x.size());
  lightCount_ = int(initial.lightDynamic.x.size());

  std::vector<float> heavyScale(heavyCount_);
  std::vector<QVector4D> heavyColor(heavyCount_);

  for (int i = 0; i < heavyCount_; ++i) {
    float visualRadius;
    if (i == 0)
      visualRadius = 0.6f; // Soleil
    else
      visualRadius = 0.05f * std::cbrt(float(metaData_[i].radius) / 6371.0f);
    visualRadius = std::max(visualRadius, 0.015f);

    heavyScale[i] = visualRadius / kSphereRadius;
    const qreal hue = heavyCount_ > 1 ? double(i) / double(heavyCount_) : 0.0;
    const QColor c = QColor::fromHsvF(hue, 0.55, 1.0);
    heavyColor[i] =
        QVector4D(float(c.redF()), float(c.greenF()), float(c.blueF()), 1.0f);
  }

  // Astéroïdes : 1/4 de la taille visuelle du Soleil (index 0).
  // Suppose que les deux Model QML ont la même échelle (voir Main.qml).
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
    if (lastAdvanceMs_ >= 0)
      advanceIntervalMs_ = now - lastAdvanceMs_;
    lastAdvanceMs_ = now;
  }

  double alpha = 1.0;
  if (advanceIntervalMs_ > 0 && lastAdvanceMs_ >= 0) {
    alpha = double(now - lastAdvanceMs_) / double(advanceIntervalMs_);
    alpha = std::clamp(alpha, 0.0, 1.0);
  }

  const StateSnapshot &prev = exchange_.prev();
  const StateSnapshot &target = exchange_.target();

  interpolate(prev.heavyDynamic, target.heavyDynamic, alpha, heavyX_, heavyY_,
              heavyZ_);
  interpolate(prev.lightDynamic, target.lightDynamic, alpha, lightX_, lightY_,
              lightZ_);

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
    outX[i] = float(x * kPositionScale);
    outY[i] = float(y * kPositionScale);
    outZ[i] = float(z * kPositionScale);
  }
}

QString FrontManager::heavyName(int index) const {
  if (index < 0 || index >= int(metaData_.size()))
    return QString();
  return QString::fromStdString(metaData_[index].name);
}

QVector3D FrontManager::heavyPosition(int index) const {
  if (index < 0 || index >= heavyCount_)
    return QVector3D();
  return QVector3D(heavyX_[index], heavyY_[index], heavyZ_[index]);
}