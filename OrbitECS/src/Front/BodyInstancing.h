#ifndef BODYINSTANCING_H
#define BODYINSTANCING_H

#include <QQuick3DInstancing>
#include <QVector4D>
#include <vector>

class BodyInstancing : public QQuick3DInstancing {
  Q_OBJECT
  Q_PROPERTY(int instanceCount READ instanceCount NOTIFY instanceCountChanged)

public:
  explicit BodyInstancing(QQuick3DObject *parent = nullptr);

  void setBodies(const std::vector<float> &x, const std::vector<float> &y,
                 const std::vector<float> &z, const std::vector<float> &scale,
                 const std::vector<QVector4D> &colorRgba);

  void updatePositions(const std::vector<float> &x,
                        const std::vector<float> &y,
                        const std::vector<float> &z);

  int instanceCount() const { return int(m_scale.size()); }

protected:
  QByteArray getInstanceBuffer(int *instanceCount) override;

signals:
  void instanceCountChanged();

private:
  void rebuildBuffer();

  std::vector<float> m_x, m_y, m_z, m_scale;
  std::vector<QVector4D> m_color;
  QByteArray m_buffer;
  bool m_dirty = true;
};

#endif