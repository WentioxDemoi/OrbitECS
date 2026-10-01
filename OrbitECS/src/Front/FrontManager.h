#ifndef FRONTMANAGER_H
#define FRONTMANAGER_H

#include <QElapsedTimer>
#include <QObject>
#include <QQuick3DInstancing>
#include <QTimer>
#include <QVector3D>
#include <QtCore/qtmetamacros.h>

#include <cstdint>
#include <vector>

#include "BodyInstancing.h"
#include "BodyMetaData.h"
#include "BufferExchange.h"

class FrontManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QQuick3DInstancing *heavyInstancing READ heavyInstancing CONSTANT)

  Q_PROPERTY(QQuick3DInstancing *lightInstancing READ lightInstancing CONSTANT)

  Q_PROPERTY(int heavyCount READ heavyCount CONSTANT)

  Q_PROPERTY(int lightCount READ lightCount CONSTANT)

  Q_PROPERTY(int simSpeedFactor READ simSpeedFactor WRITE setSimSpeedFactor
                 NOTIFY simSpeedFactorChanged)

  Q_PROPERTY(int dt READ dt WRITE setDt NOTIFY dtChanged)

  Q_PROPERTY(QString dateString READ dateString NOTIFY dateStringChanged)

public:
  explicit FrontManager(BufferExchange &exchange,
                        std::vector<BodyMetaData> metaData, QObject *parent);

  QQuick3DInstancing *heavyInstancing() { return &heavyInstancing_; }

  QQuick3DInstancing *lightInstancing() { return &lightInstancing_; }

  int heavyCount() const { return static_cast<int>(heavyCount_); }
  int lightCount() const { return static_cast<int>(lightCount_); }

  int simSpeedFactor() const { return static_cast<int>(simSpeedFactor_); }
  int dt() const { return static_cast<int>(dt_); }
  QString dateString() const { return dateString_; }

  void setDt(int dt) {
    dt_ = static_cast<uint16_t>(dt);
    emit dtChanged(dt);
  }

  void setSimSpeedFactor(int simSpeedFactor) {
    simSpeedFactor_ = static_cast<int64_t>(simSpeedFactor);
    emit simSpeedFactorChanged(simSpeedFactor);
  }

  Q_INVOKABLE QVector3D heavyPosition(int index) const;
  Q_INVOKABLE QString heavyName(int index) const;

  Q_INVOKABLE void userSetDt(int dt) { setDt(dt); }

public slots:
  void start();

private slots:
  void tick();

signals:
  void simSpeedFactorChanged(int simSpeedFactor);
  void dtChanged(int dt);
  void dateStringChanged();

private:
  void interpolate(const Dynamic &prev, const Dynamic &target, double alpha,
                   std::vector<float> &outX, std::vector<float> &outY,
                   std::vector<float> &outZ) const;

  void updateTimeStamp(const StateSnapshot &prev, const StateSnapshot &target, const uint8_t &alpha);

  BufferExchange &exchange_;
  std::vector<BodyMetaData> metaData_;

  BodyInstancing heavyInstancing_;
  BodyInstancing lightInstancing_;

  uint32_t heavyCount_ = 0;
  uint32_t lightCount_ = 0;

  int64_t simSpeedFactor_ = 1;
  uint16_t dt_ = 1;

  QTimer timer_;
  QElapsedTimer clock_;
  QString dateString_;

  qint64 lastAdvanceMs_ = -1;
  qint64 advanceIntervalMs_ = 0;

  std::vector<float> heavyX_;
  std::vector<float> heavyY_;
  std::vector<float> heavyZ_;

  std::vector<float> lightX_;
  std::vector<float> lightY_;
  std::vector<float> lightZ_;
};

#endif