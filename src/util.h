#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "types.h"

namespace fs = std::filesystem;

namespace util {

enum class ByteOrder {
	Big,
	Little,
};

u16 bswap16(u16 value);
u32 bswap32(u32 value);

bool is_equal(std::string str1, std::string str2);
s32 read_file(const fs::path& filename, std::vector<u8>& contents);
void write_file(const fs::path& filename, const std::vector<u8>& contents);
} // namespace util

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
} // namespace reader

namespace writer {
void write_u8(std::vector<u8>& buffer, size_t offset, u8 value);
void write_u16(std::vector<u8>& buffer, size_t offset, u16 value, util::ByteOrder byteOrder);
void write_u24(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void write_u32(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void write_s32(std::vector<u8>& buffer, size_t offset, s32 value, util::ByteOrder byteOrder);
void write_f32(std::vector<u8>& buffer, size_t offset, f32 value, util::ByteOrder byteOrder);
void write_u64(std::vector<u8>& buffer, size_t offset, u64 value, util::ByteOrder byteOrder);
void write_s64(std::vector<u8>& buffer, size_t offset, s64 value, util::ByteOrder byteOrder);
void write_f64(std::vector<u8>& buffer, size_t offset, f64 value, util::ByteOrder byteOrder);

void write_u16_be(std::vector<u8>& buffer, size_t offset, u16 value);
void write_u16_le(std::vector<u8>& buffer, size_t offset, u16 value);
void write_u24_be(std::vector<u8>& buffer, size_t offset, u32 value);
void write_u24_le(std::vector<u8>& buffer, size_t offset, u32 value);
void write_u32_be(std::vector<u8>& buffer, size_t offset, u32 value);
void write_u32_le(std::vector<u8>& buffer, size_t offset, u32 value);
void write_s32_be(std::vector<u8>& buffer, size_t offset, s32 value);
void write_s32_le(std::vector<u8>& buffer, size_t offset, s32 value);
void write_f32_be(std::vector<u8>& buffer, size_t offset, f32 value);
void write_f32_le(std::vector<u8>& buffer, size_t offset, f32 value);
void write_u64_be(std::vector<u8>& buffer, size_t offset, u64 value);
void write_u64_le(std::vector<u8>& buffer, size_t offset, u64 value);
void write_s64_be(std::vector<u8>& buffer, size_t offset, s64 value);
void write_s64_le(std::vector<u8>& buffer, size_t offset, s64 value);
void write_f64_be(std::vector<u8>& buffer, size_t offset, f64 value);
void write_f64_le(std::vector<u8>& buffer, size_t offset, f64 value);

void write_string(std::vector<u8>& buffer, size_t offset, const std::string& str);
void write_bytes(std::vector<u8>& buffer, size_t offset, const std::vector<u8>& bytes);
} // namespace writer

#endif
