#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using size_t = std::size_t;

using f32 = float;
using f64 = double;

typedef s32 result_t;

template <typename T>
struct Vector2 {
	T x, y;

    bool isEqual(const Vector2<T>& other) {
        return x == other.x && y == other.y;
    }
};

using Vector2f = Vector2<f32>;

template <typename T>
struct Vector3 {
	T x, y, z;

    bool isEqual(const Vector3<T>& other) {
        return x == other.x && y == other.y && z == other.z;
    }
};

using Vector3f = Vector3<f32>;

#endif
