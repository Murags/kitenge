#pragma once

#include <cmath>

namespace kitenge {

// A 2D point or direction in floating-point coordinates. Motif geometry and
// transforms work in Vec2; only the rasterisers round to integer pixels.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }

    Vec2& operator+=(Vec2 o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    Vec2& operator-=(Vec2 o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    constexpr bool operator==(Vec2 o) const { return x == o.x && y == o.y; }
    constexpr bool operator!=(Vec2 o) const { return !(*this == o); }
};

constexpr Vec2 operator*(float s, Vec2 v) {
    return v * s;
}

constexpr float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

inline float length(Vec2 v) {
    return std::sqrt(dot(v, v));
}

// Linear interpolation: t = 0 gives a, t = 1 gives b.
constexpr Vec2 lerp(Vec2 a, Vec2 b, float t) {
    return a + (b - a) * t;
}

}  // namespace kitenge
