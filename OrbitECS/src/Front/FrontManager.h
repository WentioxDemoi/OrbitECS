#ifndef FRONTMANAGER_H
#define FRONTMANAGER_H

#include <QElapsedTimer>
#include <QObject>
#include <QQuick3DInstancing>
#include <QString>
#include <QTimer>
#include <QVector3D>

#include <cstdint>
#include <vector>

#include "BodyInstancing.h"
#include "BodyMetaData.h"
#include "BufferExchange.h"

using f32 = float;
using f64 = double;

static_assert(sizeof(f32) == 4, "f32 must be 32-bit");
static_assert(sizeof(f64) == 8, "f64 must be 64-bit");
static_assert(sizeof(qint32) == sizeof(std::int32_t), "qint32 mismatch");

class FrontManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QQuick3DInstancing *heavyInstancing READ heavyInstancing CONSTANT)
  Q_PROPERTY(QQuick3DInstancing *lightInstancing READ lightInstancing CONSTANT)

  Q_PROPERTY(qint32 heavyCount READ heavyCount CONSTANT)
  Q_PROPERTY(qint32 lightCount READ lightCount CONSTANT)

  Q_PROPERTY(qint32 simSpeedFactor READ simSpeedFactor WRITE setSimSpeedFactor
                 NOTIFY simSpeedFactorChanged)

  Q_PROPERTY(qint32 dt READ dt WRITE setDt NOTIFY dtChanged)

  Q_PROPERTY(QString dateString READ dateString NOTIFY dateStringChanged)

public:
  explicit FrontManager(BufferExchange &exchange,
                        std::vector<BodyMetaData> metaData,
                        QObject *parent = nullptr);

  QQuick3DInstancing *heavyInstancing() { return &heavyInstancing_; }
  QQuick3DInstancing *lightInstancing() { return &lightInstancing_; }

  qint32 heavyCount() const { return static_cast<qint32>(heavyCount_); }
  qint32 lightCount() const { return static_cast<qint32>(lightCount_); }

  qint32 simSpeedFactor() const { return simSpeedFactor_; }
  qint32 dt() const { return static_cast<qint32>(dt_); }

  QString dateString() const { return dateString_; }

  void setDt(qint32 dt);
  void setSimSpeedFactor(qint32 simSpeedFactor);

  Q_INVOKABLE QVector3D heavyPosition(qint32 index) const;
  Q_INVOKABLE QString heavyName(qint32 index) const;
  Q_INVOKABLE QString heavyText(qint32 index) const;

  Q_INVOKABLE void userSetDt(qint32 dt) { setDt(dt); }

public slots:
  void start();

private slots:
  void tick();

signals:
  void simSpeedFactorChanged(qint32 simSpeedFactor);
  void dtChanged(qint32 dt);
  void dateStringChanged();

private:
  void interpolate(const Dynamic &prev, const Dynamic &target, f64 alpha,
                   std::vector<f32> &outX, std::vector<f32> &outY,
                   std::vector<f32> &outZ) const;

  void updateTimeStamp(const StateSnapshot &prev, const StateSnapshot &target,
                       f64 alpha);

  BufferExchange &exchange_;
  std::vector<BodyMetaData> metaData_;

  BodyInstancing heavyInstancing_;
  BodyInstancing lightInstancing_;

  std::uint32_t heavyCount_ = 0;
  std::uint32_t lightCount_ = 0;

  std::int32_t simSpeedFactor_ = 1;
  std::uint16_t dt_ = 1;

  QTimer timer_;
  QElapsedTimer clock_;
  QString dateString_;

  std::int64_t lastAdvanceMs_ = -1;
  std::int64_t advanceIntervalMs_ = 0;

  std::vector<f32> heavyX_;
  std::vector<f32> heavyY_;
  std::vector<f32> heavyZ_;

  std::vector<f32> lightX_;
  std::vector<f32> lightY_;
  std::vector<f32> lightZ_;
};

#endif