#include "BufferExchange.h"

void BufferExchange::publish() {
    std::scoped_lock lock(mutex_);
    storage_[Write].swap(storage_[Ready]);
    hasNewData_ = true;
}

bool BufferExchange::tryAdvance() {
    std::scoped_lock lock(mutex_);
    if (!hasNewData_) return false;
    storage_[Prev].swap(storage_[Target]);
    storage_[Target].swap(storage_[Ready]);
    hasNewData_ = false;
    return true;
}