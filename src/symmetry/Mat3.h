#pragma once

#include <cmath>

#include "core/Vec2.h"

namespace kitenge {

// A 3x3 matrix for 2D transforms in homogeneous coordinates. A point (x, y)
// is treated as the column vector (x, y, 1), so translation, rotation, scaling
// and mirroring are all matrix products and can be chained into one matrix.
//
// Products apply right to left: (a * b).apply(p) == a.apply(b.apply(p)).
struct Mat3 {
    // m[row][col]
    float m[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    static constexpr Mat3 identity() { return Mat3(); }

    static constexpr Mat3 translate(float tx, float ty) {
        Mat3 r;
        r.m[0][2] = tx;
        r.m[1][2] = ty;
        return r;
    }

    static constexpr Mat3 scale(float sx, float sy) {
        Mat3 r;
        r.m[0][0] = sx;
        r.m[1][1] = sy;
        return r;
    }

    // Rotation by `radians` about the origin. With y pointing down on screen,
    // a positive angle turns clockwise.
    static Mat3 rotate(float radians) {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        Mat3 r;
        r.m[0][0] = c;
        r.m[0][1] = -s;
        r.m[1][0] = s;
        r.m[1][1] = c;
        return r;
    }

    // Reflection across the vertical axis (x -> -x).
    static constexpr Mat3 mirrorX() { return scale(-1.0f, 1.0f); }

    // Reflection across the horizontal axis (y -> -y).
    static constexpr Mat3 mirrorY() { return scale(1.0f, -1.0f); }

    constexpr Mat3 operator*(const Mat3& o) const {
        Mat3 r;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                r.m[i][j] = m[i][0] * o.m[0][j] + m[i][1] * o.m[1][j] + m[i][2] * o.m[2][j];
            }
        }
        return r;
    }

    constexpr Vec2 apply(Vec2 p) const {
        const float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
        const float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
        const float w = m[2][0] * p.x + m[2][1] * p.y + m[2][2];
        return {x / w, y / w};
    }
};

}  // namespace kitenge
