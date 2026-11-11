#include "symmetry/Assembly.h"

#include <algorithm>

namespace kitenge {

void AssemblyPlayer::start(int totalTiles) {
    total_ = std::max(totalTiles, 0);
    elapsed_ = 0.0f;
    visible_ = 0;
    playing_ = total_ > 0;
}

int AssemblyPlayer::update(float seconds) {
    if (!playing_) {
        return visible_;
    }
    elapsed_ += std::max(seconds, 0.0f);
    visible_ = std::min(total_, static_cast<int>(elapsed_ * rate_));
    if (visible_ >= total_) {
        playing_ = false;
    }
    return visible_;
}

}  // namespace kitenge
