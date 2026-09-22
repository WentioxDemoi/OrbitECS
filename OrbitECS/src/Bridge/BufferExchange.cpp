#include "BufferExchange.h"

void BufferExchange::publish() {
  std::scoped_lock lock(mutex_);
  lastIdx_ = idx_[Write];
  std::swap(idx_[Write], idx_[Ready]);
  hasNewData_ = true;
}

bool BufferExchange::tryAdvance() {
  std::scoped_lock lock(mutex_);
  if (!hasNewData_)
    return false;
  std::swap(idx_[Prev], idx_[Target]);
  std::swap(idx_[Target], idx_[Ready]);
  hasNewData_ = false;
  return true;
}