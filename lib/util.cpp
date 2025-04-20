#include "util.h"

#include <bit>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace util {
u16 bswap16(u16 value) {
	return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
}

u32 bswap32(u32 value) {
	return ((value & 0x000000ff) << 24) | ((value & 0x0000ff00) << 8) |
	       ((value & 0x00ff0000) >> 8) | ((value & 0xff000000) >> 24);
}

bool is_equal(std::string str1, std::string str2) {
	return std::strcmp(str1.c_str(), str2.c_str()) == 0;
}

u32 round_up(u32 x, u32 power_of_2) {
	u32 a = power_of_2 - 1;
	return (x + a) & ~a;
}

result_t read_file(std::vector<u8>& contents, const fs::path& filename) {
	std::ifstream fstream(filename, std::ios::binary);

	if (fstream.eof() || fstream.fail()) {
		return Error::FileError;
	}

	// disable skipping whitespace in binary file
	fstream.unsetf(std::ios::skipws);

	fstream.seekg(0, std::ios_base::end);
	std::streampos fileSize = fstream.tellg();
	fstream.seekg(0, std::ios_base::beg);

	contents.reserve(fileSize);
	contents.insert(
		contents.begin(), std::istream_iterator<u8>(fstream), std::istream_iterator<u8>()
	);

	fstream.close();

	return 0;
}

void write_file(const fs::path& filename, const std::vector<u8>& contents) {
	std::ofstream fstream(filename, std::ios::out | std::ios::binary);
	fstream.write(reinterpret_cast<const char*>(contents.data()), contents.size());
}

} // namespace util

namespace reader {
result_t read_byte_order(util::ByteOrder* out, const u8* offset, u16 expected_be) {
	u16 mark = (offset[0] << 8) | offset[1];
	if (mark == expected_be)
		*out = util::ByteOrder::Big;
	else if (mark == util::bswap16(expected_be))
		*out = util::ByteOrder::Little;
	else
		return Error::BadByteOrder;

	return 0;
}

result_t check_signature(const u8* offset, const std::string& expected, size_t length) {
	for (size_t i = 0; i < length; i++)
		if (offset[i] != expected[i]) return Error::BadSignature;

	return 0;
}

u8 read_u8(const u8* offset) {
	return *offset;
}

u16 read_u16(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return (offset[0] << 8) | (offset[1] << 0);
	else
		return (offset[0] << 0) | (offset[1] << 8);
}

u32 read_u24(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return (offset[0] << 16) | (offset[1] << 8) | (offset[2] << 0);
	else
		return (offset[0] << 0) | (offset[1] << 8) | (offset[2] << 16);
}

u32 read_u32(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return (offset[0] << 24) | (offset[1] << 16) | (offset[2] << 8) | offset[3];
	else
		return offset[0] | (offset[1] << 8) | (offset[2] << 16) | (offset[3] << 24);
}

s32 read_s32(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<s32>(read_u32(offset, byteOrder));
}

f32 read_f32(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<f32>(read_u32(offset, byteOrder));
}

u64 read_u64(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return ((u64)offset[0] << 56) | ((u64)offset[1] << 48) | ((u64)offset[2] << 40) |
		       ((u64)offset[3] << 32) | ((u64)offset[4] << 24) | ((u64)offset[5] << 16) |
		       ((u64)offset[6] << 8) | ((u64)offset[7] << 0);
	else
		return ((u64)offset[0] << 0) | ((u64)offset[1] << 8) | ((u64)offset[2] << 16) |
		       ((u64)offset[3] << 24) | ((u64)offset[4] << 32) | ((u64)offset[5] << 40) |
		       ((u64)offset[6] << 48) | ((u64)offset[7] << 56);
}

s64 read_s64(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<s64>(read_u64(offset, byteOrder));
}

f64 read_f64(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<f64>(read_u64(offset, byteOrder));
}

std::string read_string(const u8* offset) {
	std::string str((const char*)offset);
	return str;
}

std::string read_string(const u8* offset, size_t length) {
	std::string str((const char*)offset, length);
	return str;
}

std::vector<u8> read_bytes(const u8* offset, size_t size) {
	std::vector<u8> section(offset, offset + size);
	return section;
}
} // namespace reader

namespace writer {
void write_u8(std::vector<u8>& buffer, size_t offset, u8 value) {
	if (offset + 1 > buffer.size()) buffer.resize(offset + 1);

	buffer[offset] = value;
}

void write_u16_be(std::vector<u8>& buffer, size_t offset, u16 value) {
	if (offset + 2 > buffer.size()) buffer.resize(offset + 2);

	buffer[offset + 0] = value >> 8 & 0xff;
	buffer[offset + 1] = value >> 0 & 0xff;
}

void write_u16_le(std::vector<u8>& buffer, size_t offset, u16 value) {
	if (offset + 2 > buffer.size()) buffer.resize(offset + 2);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
}

void write_u16(std::vector<u8>& buffer, size_t offset, u16 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		write_u16_be(buffer, offset, value);
	else
		write_u16_le(buffer, offset, value);
}

void write_u24_be(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 3 > buffer.size()) buffer.resize(offset + 3);

	buffer[offset + 0] = value >> 16 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 0 & 0xff;
}

void write_u24_le(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 3 > buffer.size()) buffer.resize(offset + 3);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
}

void write_u24(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		write_u24_be(buffer, offset, value);
	else
		write_u24_le(buffer, offset, value);
}

void write_u32_be(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 4 > buffer.size()) buffer.resize(offset + 4);

	buffer[offset + 0] = value >> 24 & 0xff;
	buffer[offset + 1] = value >> 16 & 0xff;
	buffer[offset + 2] = value >> 8 & 0xff;
	buffer[offset + 3] = value >> 0 & 0xff;
}

void write_u32_le(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 4 > buffer.size()) buffer.resize(offset + 4);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
	buffer[offset + 3] = value >> 24 & 0xff;
}

void write_u32(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		write_u32_be(buffer, offset, value);
	else
		write_u32_le(buffer, offset, value);
}

void write_s32_be(std::vector<u8>& buffer, size_t offset, s32 value) {
	write_u32_be(buffer, offset, std::bit_cast<u32>(value));
}

void write_s32_le(std::vector<u8>& buffer, size_t offset, s32 value) {
	write_u32_le(buffer, offset, std::bit_cast<u32>(value));
}

void write_s32(std::vector<u8>& buffer, size_t offset, s32 value, util::ByteOrder byteOrder) {
	write_u32(buffer, offset, std::bit_cast<u32>(value), byteOrder);
}

void write_f32_be(std::vector<u8>& buffer, size_t offset, f32 value) {
	write_u32_be(buffer, offset, std::bit_cast<u32>(value));
}

void write_f32_le(std::vector<u8>& buffer, size_t offset, f32 value) {
	write_u32_le(buffer, offset, std::bit_cast<u32>(value));
}

void write_f32(std::vector<u8>& buffer, size_t offset, f32 value, util::ByteOrder byteOrder) {
	write_u32(buffer, offset, std::bit_cast<u32>(value), byteOrder);
}

void write_u64_be(std::vector<u8>& buffer, size_t offset, u64 value) {
	if (offset + 8 > buffer.size()) buffer.resize(offset + 8);

	buffer[offset + 0] = value >> 56 & 0xff;
	buffer[offset + 1] = value >> 48 & 0xff;
	buffer[offset + 2] = value >> 40 & 0xff;
	buffer[offset + 3] = value >> 32 & 0xff;
	buffer[offset + 4] = value >> 24 & 0xff;
	buffer[offset + 5] = value >> 16 & 0xff;
	buffer[offset + 6] = value >> 8 & 0xff;
	buffer[offset + 7] = value >> 0 & 0xff;
}

void write_u64_le(std::vector<u8>& buffer, size_t offset, u64 value) {
	if (offset + 8 > buffer.size()) buffer.resize(offset + 8);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
	buffer[offset + 3] = value >> 24 & 0xff;
	buffer[offset + 4] = value >> 32 & 0xff;
	buffer[offset + 5] = value >> 40 & 0xff;
	buffer[offset + 6] = value >> 48 & 0xff;
	buffer[offset + 7] = value >> 56 & 0xff;
}

void write_u64(std::vector<u8>& buffer, size_t offset, u64 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		write_u64_be(buffer, offset, value);
	else
		write_u64_le(buffer, offset, value);
}

void write_s64_be(std::vector<u8>& buffer, size_t offset, s64 value) {
	write_u64_be(buffer, offset, std::bit_cast<u64>(value));
}

void write_s64_le(std::vector<u8>& buffer, size_t offset, s64 value) {
	write_u64_le(buffer, offset, std::bit_cast<u64>(value));
}

void write_s64(std::vector<u8>& buffer, size_t offset, s64 value, util::ByteOrder byteOrder) {
	write_u64(buffer, offset, std::bit_cast<u64>(value), byteOrder);
}

void write_f64_be(std::vector<u8>& buffer, size_t offset, f64 value) {
	write_u64_be(buffer, offset, std::bit_cast<u64>(value));
}

void write_f64_le(std::vector<u8>& buffer, size_t offset, f64 value) {
	write_u64_le(buffer, offset, std::bit_cast<u64>(value));
}

void write_f64(std::vector<u8>& buffer, size_t offset, f64 value, util::ByteOrder byteOrder) {
	write_u64(buffer, offset, std::bit_cast<u64>(value), byteOrder);
}

void write_string(std::vector<u8>& buffer, size_t offset, const std::string& str) {
	for (size_t i = 0; i < str.size(); i++)
		write_u8(buffer, offset + i, str[i]);
	write_u8(buffer, offset + str.size(), 0);
}

void write_bytes(std::vector<u8>& buffer, size_t offset, const std::vector<u8>& bytes) {
	for (size_t i = 0; i < bytes.size(); i++)
		write_u8(buffer, offset + i, bytes[i]);
}

} // namespace writer
