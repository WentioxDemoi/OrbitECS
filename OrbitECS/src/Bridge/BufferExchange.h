#ifndef BUFFEREXCHANGE_H
#define BUFFEREXCHANGE_H

#include "Components/StateSnapshot.h"
#include <array>
#include <mutex>

class BufferExchange {
public:
    BufferExchange(const Dynamic &heavy, const Dynamic &light)
        : storage_{StateSnapshot(heavy, light), StateSnapshot(heavy, light),
                   StateSnapshot(heavy, light), StateSnapshot(heavy, light)} {}

    StateSnapshot& writeSlot() noexcept { return storage_[idx_[Write]]; }
    const StateSnapshot& prev()   const noexcept { return storage_[idx_[Prev]]; }
    const StateSnapshot& target() const noexcept { return storage_[idx_[Target]]; }

    void publish();
    bool tryAdvance();

private:
    enum Role : std::size_t { Write, Ready, Target, Prev };

    std::array<StateSnapshot, 4> storage_;
    std::array<std::size_t, 4> idx_{0, 1, 2, 3};
    bool hasNewData_ = false;
    std::mutex mutex_;
};

#endif