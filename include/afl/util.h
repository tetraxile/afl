#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <string>
#include <vector>

#include "afl/types.h"

namespace fs = std::filesystem;

namespace util {

enum Error : result_t {
	BadSignature = 1,
	BadByteOrder,
	FileError,
	FileNotFound,
	DirNotFound,
};

enum class ByteOrder {
	Big,
	Little,
};

u16 bswap16(u16 value);
u32 bswap32(u32 value);

bool isEqual(std::string str1, std::string str2);
u32 roundUp(u32 x, u32 powerOf2);
s32 readFile(std::vector<u8>& contents, const fs::path& filename);
void writeFile(const fs::path& filename, const std::vector<u8>& contents);
} // namespace util

namespace reader {
result_t readByteOrder(util::ByteOrder* out, const u8* offset, u16 expectedBE);
result_t checkSignature(const u8* offset, const std::string& expected, size_t length);
u8 readU8(const u8* offset);
u16 readU16(const u8* offset, util::ByteOrder byteOrder);
u32 readU24(const u8* offset, util::ByteOrder byteOrder);
u32 readU32(const u8* offset, util::ByteOrder byteOrder);
s32 readS32(const u8* offset, util::ByteOrder byteOrder);
f32 readF32(const u8* offset, util::ByteOrder byteOrder);
u64 readU64(const u8* offset, util::ByteOrder byteOrder);
s64 readS64(const u8* offset, util::ByteOrder byteOrder);
f64 readF64(const u8* offset, util::ByteOrder byteOrder);
std::string readString(const u8* offset);
std::string readString(const u8* offset, size_t length);
std::vector<u8> readBytes(const u8* offset, size_t size);
} // namespace reader

namespace writer {
void writeU8(std::vector<u8>& buffer, size_t offset, u8 value);
void writeU16(std::vector<u8>& buffer, size_t offset, u16 value, util::ByteOrder byteOrder);
void writeU24(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void writeU32(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void writeS32(std::vector<u8>& buffer, size_t offset, s32 value, util::ByteOrder byteOrder);
void writeF32(std::vector<u8>& buffer, size_t offset, f32 value, util::ByteOrder byteOrder);
void writeU64(std::vector<u8>& buffer, size_t offset, u64 value, util::ByteOrder byteOrder);
void writeS64(std::vector<u8>& buffer, size_t offset, s64 value, util::ByteOrder byteOrder);
void writeF64(std::vector<u8>& buffer, size_t offset, f64 value, util::ByteOrder byteOrder);

void writeU16BE(std::vector<u8>& buffer, size_t offset, u16 value);
void writeU16LE(std::vector<u8>& buffer, size_t offset, u16 value);
void writeU24BE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU24LE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU32BE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU32LE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeS32BE(std::vector<u8>& buffer, size_t offset, s32 value);
void writeS32LE(std::vector<u8>& buffer, size_t offset, s32 value);
void writeF32BE(std::vector<u8>& buffer, size_t offset, f32 value);
void writeF32LE(std::vector<u8>& buffer, size_t offset, f32 value);
void writeU64BE(std::vector<u8>& buffer, size_t offset, u64 value);
void writeU64LE(std::vector<u8>& buffer, size_t offset, u64 value);
void writeS64BE(std::vector<u8>& buffer, size_t offset, s64 value);
void writeS64LE(std::vector<u8>& buffer, size_t offset, s64 value);
void writeF64BE(std::vector<u8>& buffer, size_t offset, f64 value);
void writeF64LE(std::vector<u8>& buffer, size_t offset, f64 value);

void writeString(std::vector<u8>& buffer, size_t offset, const std::string& str);
void writeBytes(std::vector<u8>& buffer, size_t offset, const std::vector<u8>& bytes);
} // namespace writer

#endif
