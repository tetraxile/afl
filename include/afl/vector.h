#pragma once

#include "afl/types.h"
#include "afl/util.h"

template <typename T>
struct Vector2 {
	T x, y;

    bool isEqual(const Vector2<T>& other) const {
        return x == other.x && y == other.y;
    }
};

using Vector2i = Vector2<s32>;
using Vector2f = Vector2<f32>;

template <typename T>
struct Vector3 {
	T x, y, z;

    bool isEqual(const Vector3<T>& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

using Vector3i = Vector3<s32>;
using Vector3f = Vector3<f32>;

template <typename T>
struct std::hash<Vector2<T>> {
    std::size_t operator()(const Vector2<T>& vec) const noexcept {
        size_t out = 0;
        util::hashCombine(out, vec.x);
        util::hashCombine(out, vec.y);
        return out;
    }
};

template <typename T>
struct std::hash<Vector3<T>> {
    std::size_t operator()(const Vector3<T>& vec) const noexcept {
        size_t out = 0;
        util::hashCombine(out, vec.x);
        util::hashCombine(out, vec.y);
        util::hashCombine(out, vec.z);
        return out;
    }
};
