#ifndef UTIL_H
#define UTIL_H

#include "types.h"

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

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

namespace util {

enum class ByteOrder {
	Big,
	Little,
};

u16 bswap16(u16 value);
u32 bswap32(u32 value);

void write_u16_be(std::vector<u8>& buffer, size_t offset, u16 value);
void write_u32_be(std::vector<u8>& buffer, size_t offset, u32 value);

bool is_equal(std::string str1, std::string str2);
s32 read_file(const fs::path& filename, std::vector<u8>& contents);
void write_file(const fs::path& filename, const std::vector<u8>& contents);
}

namespace reader {
result_t read_byte_order(util::ByteOrder* out, const u8* offset, u16 expected_be);
result_t check_signature(const u8* offset, const std::string& expected, size_t length);
u8 read_u8(const u8* offset);
u16 read_u16(const u8* offset, util::ByteOrder byteOrder);
u32 read_u24(const u8* offset, util::ByteOrder byteOrder);
u32 read_u32(const u8* offset, util::ByteOrder byteOrder);
s32 read_s32(const u8* offset, util::ByteOrder byteOrder);
f32 read_f32(const u8* offset, util::ByteOrder byteOrder);
u64 read_u64(const u8* offset, util::ByteOrder byteOrder);
s64 read_s64(const u8* offset, util::ByteOrder byteOrder);
f64 read_f64(const u8* offset, util::ByteOrder byteOrder);
std::string read_string(const u8* offset);
std::string read_string(const u8* offset, size_t length);
std::vector<u8> read_bytes(const u8* offset, size_t size);
}

#endif
