#pragma once

#include <cstddef>
#include <cstdint>

#include "float16_t/float16_t.hpp"

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using size_t = std::size_t;

using f16 = numeric::float16_t;
using f32 = float;
using f64 = double;

typedef s32 result_t;
