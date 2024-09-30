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

class BinaryReader {
public:
	BinaryReader(BinaryReader& a) : mBuffer(a.mBuffer){}
	BinaryReader(const std::vector<u8>& buffer) : mBuffer(buffer){}

	u8 read_u8();
	u16 read_u16();
	u32 read_u24();
	u32 read_u32();
	u64 read_u64();
	result_t check_signature(const char* expected, size_t length);
	result_t read_byte_order(u16 expected_be = 0xfeff);
	std::vector<u8> read_bytes(size_t size);
	std::string read_string();

	void seek(size_t offset);
	void seek_rel(size_t offset);

	size_t pos() const { return mCursor; }

private:
	const std::vector<u8>& mBuffer;
	size_t mCursor = 0;
	ByteOrder mByteOrder = ByteOrder::Big;
};

u16 bswap16(u16 value);
u32 bswap32(u32 value);

u16 read_u16_be(const std::vector<u8>& buffer, size_t offset);
u32 read_u32_be(const std::vector<u8>& buffer, size_t offset);
u32 read_u32_le(const std::vector<u8>& buffer, size_t offset);

void write_u16_be(std::vector<u8>& buffer, size_t offset, u16 value);
void write_u32_be(std::vector<u8>& buffer, size_t offset, u32 value);

bool is_equal(std::string str1, std::string str2);
s32 read_file(const fs::path& filename, std::vector<u8>& contents);
void write_file(const fs::path& filename, const std::vector<u8>& contents);
}

#endif
