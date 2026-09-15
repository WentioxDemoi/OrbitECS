#include "BufferExchange.h"

void BufferExchange::publish() {
    std::scoped_lock lock(mutex_);
    std::swap(write_, ready_);   // le buffer qu'on vient de remplir devient "ready"
    hasNewData_ = true;          // "ready_" pointe maintenant vers l'ancien writeSlot
}

bool BufferExchange::tryAdvance() {
    std::scoped_lock lock(mutex_);
    if (!hasNewData_) return false;
    std::swap(prev_, target_);   // l'ancien target devient prev
    std::swap(target_, ready_);  // ready devient le nouveau target,
                                  // et l'ancien prev (maintenant dans ready_)
                                  // sera récupéré par publish() au prochain tour
    hasNewData_ = false;
    return true;
}