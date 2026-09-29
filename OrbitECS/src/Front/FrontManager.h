#ifndef FRONTMANAGER_H
#define FRONTMANAGER_H

#include <QElapsedTimer>
#include <QObject>
#include <QQuick3DInstancing>
#include <QTimer>
#include <QVector3D>
#include <QtCore/qtmetamacros.h>
#include <iostream>
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

public:
  explicit FrontManager(BufferExchange &exchange,
                        std::vector<BodyMetaData> metaData, QObject *parent);

  QQuick3DInstancing *heavyInstancing() { return &heavyInstancing_; }
  QQuick3DInstancing *lightInstancing() { return &lightInstancing_; }

  int heavyCount() const { return heavyCount_; }
  int lightCount() const { return lightCount_; }
  int simSpeedFactor() const { return simSpeedFactor_; }
  int dt() const { return dt_; }

  void setDt(int dt) {
    dt_ = dt;
    emit dtChanged(dt_);
  }
  void setSimSpeedFactor(int simSpeedFactor) {
    simSpeedFactor_ = simSpeedFactor;
    emit simSpeedFactorChanged(simSpeedFactor_);
  }

  Q_INVOKABLE QVector3D heavyPosition(int index) const;
  Q_INVOKABLE QString heavyName(int index) const;

public slots:
  void start();

private slots:
  void tick();

signals:
  void simSpeedFactorChanged(int simSpeedFactor);
  void dtChanged(int dt);

private:
  void interpolate(const Dynamic &prev, const Dynamic &target, double alpha,
                   std::vector<float> &outX, std::vector<float> &outY,
                   std::vector<float> &outZ) const;

  BufferExchange &exchange_;
  std::vector<BodyMetaData> metaData_;

  BodyInstancing heavyInstancing_;
  BodyInstancing lightInstancing_;

  int heavyCount_ = 0;
  int lightCount_ = 0;

  int simSpeedFactor_ = 1;
  int dt_ = 1;

  QTimer timer_;
  QElapsedTimer clock_;
  qint64 lastAdvanceMs_ = -1;
  qint64 advanceIntervalMs_ = 0;

  std::vector<float> heavyX_, heavyY_, heavyZ_;
  std::vector<float> lightX_, lightY_, lightZ_;
};

#endif