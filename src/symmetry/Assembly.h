#pragma once

namespace kitenge {

// Plays back the pattern assembling itself: copies of the motif appear one
// after another, in the order the layout placed them.
class AssemblyPlayer {
public:
    // Copies revealed per second.
    explicit AssemblyPlayer(float tilesPerSecond = 12.0f) : rate_(tilesPerSecond) {}

    // Starts from an empty canvas towards `totalTiles` copies.
    void start(int totalTiles);

    void stop() { playing_ = false; }

    bool playing() const { return playing_; }

    // Advances by `seconds` and returns how many copies are visible now.
    // Stops by itself once every copy is shown.
    int update(float seconds);

    int visible() const { return visible_; }
    int total() const { return total_; }

private:
    float rate_;
    float elapsed_ = 0.0f;
    int total_ = 0;
    int visible_ = 0;
    bool playing_ = false;
};

}  // namespace kitenge
