#include "BodyInstancing.h"
#include <QColor>

BodyInstancing::BodyInstancing(QQuick3DObject *parent)
    : QQuick3DInstancing(parent) {}

void BodyInstancing::setBodies(const std::vector<float> &x,
                               const std::vector<float> &y,
                               const std::vector<float> &z,
                               const std::vector<float> &scale,
                               const std::vector<QVector4D> &colorRgba) {
  const bool countChanged = scale.size() != m_scale.size();
  m_x = x;
  m_y = y;
  m_z = z;
  m_scale = scale;
  m_color = colorRgba;
  m_dirty = true;
  markDirty();
  if (countChanged)
    emit instanceCountChanged();
}

void BodyInstancing::updatePositions(const std::vector<float> &x,
                                     const std::vector<float> &y,
                                     const std::vector<float> &z) {
  m_x = x;
  m_y = y;
  m_z = z;
  m_dirty = true;
  markDirty();
}

void BodyInstancing::rebuildBuffer() {
  const int count = int(m_scale.size());
  m_buffer.resize(count * int(sizeof(InstanceTableEntry)));
  auto *entries = reinterpret_cast<InstanceTableEntry *>(m_buffer.data());

  for (int i = 0; i < count; ++i) {
    const QVector3D pos(m_x[i], m_y[i], m_z[i]);
    const QVector3D scl(m_scale[i], m_scale[i], m_scale[i]);
    const QColor color = QColor::fromRgbF(m_color[i].x(), m_color[i].y(),
                                          m_color[i].z(), m_color[i].w());
    entries[i] = calculateTableEntry(pos, scl, QVector3D(0, 0, 0), color);
  }
  m_dirty = false;
}

QByteArray BodyInstancing::getInstanceBuffer(int *instanceCount) {
  if (m_dirty)
    rebuildBuffer();
  if (instanceCount)
    *instanceCount = int(m_scale.size());
  return m_buffer;
}