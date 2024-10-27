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

enum Error : result_t {
	BadSignature = 1,
	BadByteOrder,
	FileError,
};

constexpr const char* result_to_string(result_t r) {
	switch (r) {
	case Error::BadSignature: return "bad signature";
	case Error::BadByteOrder: return "invalid byte order";
	case Error::FileError: return "file error";
	}
	return "(unknown)";
}

#endif
