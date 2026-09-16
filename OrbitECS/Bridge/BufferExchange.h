#include "Components/StateSnapshot.h"
#include <array>
#include <mutex>

class BufferExchange {
public:
    explicit BufferExchange(std::size_t heavyCount, std::size_t lightCount)
    : storage_{StateSnapshot(heavyCount, lightCount), StateSnapshot(heavyCount, lightCount),
                StateSnapshot(heavyCount, lightCount), StateSnapshot(heavyCount, lightCount)}
    {}

    StateSnapshot& writeSlot() noexcept { return storage_[Write]; }

    void publish();
    bool tryAdvance();

    const StateSnapshot& prev()   const noexcept { return storage_[Prev]; }
    const StateSnapshot& target() const noexcept { return storage_[Target]; }

private:
    enum Slot : std::size_t { Write = 0, Ready = 1, Target = 2, Prev = 3 };

    std::array<StateSnapshot, 4> storage_;
    bool hasNewData_ = false;
    std::mutex mutex_;
};