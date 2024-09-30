#include "util.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace util {

u8 BinaryReader::read_u8() {
	return mBuffer[mCursor++];
}

// idea:
//
// u8 BinaryReader::read_u8_unsafe() {
//	return mBuffer[mCursor++];
// }
// 
// u8 BinaryReader::read_u8(bool* success) {
// 	if (*success == false || mByteOrder == ByteOrder::Invalid || mCursor >= mBuffer.size()) {
// 		*success = false;
// 		return 0;
// 	}
//
// 	return read_u8_unsafe();
// }
//
// bool* success = { true };
// u8 a = read_u8(success);
// u8 b = read_u8(success);
// if (!*success) printf("error :(\n");

u16 BinaryReader::read_u16() {
	mCursor += 2;
	if (mByteOrder == ByteOrder::Big)
		return (mBuffer[mCursor - 2] << 8) | mBuffer[mCursor - 1];
	else
		return mBuffer[mCursor - 2] | (mBuffer[mCursor - 1] << 8);
}

u32 BinaryReader::read_u24() {
	mCursor += 3;
	if (mByteOrder == ByteOrder::Big)
		return (mBuffer[mCursor - 3] << 16) |
			   (mBuffer[mCursor - 2] <<  8) |
			   (mBuffer[mCursor - 1] <<  0);
	else
		return (mBuffer[mCursor - 3] <<  0) |
			   (mBuffer[mCursor - 2] <<  8) |
			   (mBuffer[mCursor - 1] << 16);
}

u32 BinaryReader::read_u32() {
	mCursor += 4;
	if (mByteOrder == ByteOrder::Big)
		return (mBuffer[mCursor - 4] << 24) |
			   (mBuffer[mCursor - 3] << 16) |
			   (mBuffer[mCursor - 2] <<  8) |
			   (mBuffer[mCursor - 1] <<  0);
	else
		return (mBuffer[mCursor - 4] <<  0) |
			   (mBuffer[mCursor - 3] <<  8) |
			   (mBuffer[mCursor - 2] << 16) |
			   (mBuffer[mCursor - 1] << 24);
}

u64 BinaryReader::read_u64() {
	mCursor += 8;
	// if (mByteOrder == ByteOrder::Big)
	// 	return (mBuffer[mCursor - 8] << 56) |
	// 		   (mBuffer[mCursor - 7] << 48) |
	// 		   (mBuffer[mCursor - 6] << 40) |
	// 		   (mBuffer[mCursor - 5] << 32) |
	// 		   (mBuffer[mCursor - 4] << 24) |
	// 		   (mBuffer[mCursor - 3] << 16) |
	// 		   (mBuffer[mCursor - 2] <<  8) |
	// 		   (mBuffer[mCursor - 1] <<  0);
	// else if (mByteOrder == ByteOrder::Little)
	// 	return (mBuffer[mCursor - 8] <<  0) |
	// 		   (mBuffer[mCursor - 7] <<  8) |
	// 		   (mBuffer[mCursor - 6] << 16) |
	// 		   (mBuffer[mCursor - 5] << 24) |
	// 		   (mBuffer[mCursor - 4] << 32) |
	// 		   (mBuffer[mCursor - 3] << 40) |
	// 		   (mBuffer[mCursor - 2] << 48) |
	// 		   (mBuffer[mCursor - 1] << 56);
	return 0;
}

result_t BinaryReader::check_signature(const char* expected, size_t length) {
	for (size_t i = 0; i < length; i++) {
		if (mBuffer[mCursor + i] != expected[i]) {
			mCursor += length;
			return Error::BadSignature;
		}
	}

	mCursor += length;
	return 0;
}

result_t BinaryReader::read_byte_order(u16 expected_be) {
	mCursor += 2;
	u16 mark = (mBuffer[mCursor - 2] << 8) | mBuffer[mCursor - 1];
	if (mark == expected_be)
		mByteOrder = ByteOrder::Big;
	else if (mark == bswap16(expected_be))
		mByteOrder = ByteOrder::Little;
	else
		return Error::BadByteOrder;

	return 0;
}

void BinaryReader::seek(size_t offset) {
	mCursor = offset;
}

void BinaryReader::seek_rel(size_t offset) {
	mCursor += offset;
}

std::vector<u8> BinaryReader::read_bytes(size_t size) {
	auto iter = mBuffer.begin() + mCursor;
	std::vector<u8> section(iter, iter + size);
	mCursor += size;
	return section;
}

std::string BinaryReader::read_string() {
	auto iter = mBuffer.begin() + mCursor;
	std::string str(iter, mBuffer.end());
	mCursor += str.size();
	return str;
}

u16 bswap16(u16 value) {
	return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
}

u32 bswap32(u32 value) {
	return ((value & 0x000000ff) << 24) | ((value & 0x0000ff00) <<  8) |
		   ((value & 0x00ff0000) >>  8) | ((value & 0xff000000) >> 24);
}

u16 read_u16_be(const std::vector<u8>& buffer, size_t offset) {
	return (buffer[offset] << 8) | buffer[offset+1];
}

u32 read_u32_be(const std::vector<u8>& buffer, size_t offset) {
	return (buffer[offset+0] << 24) |
	       (buffer[offset+1] << 16) |
	       (buffer[offset+2] <<  8) |
		   (buffer[offset+3] <<  0);
}

u32 read_u32_le(const std::vector<u8>& buffer, size_t offset) {
	return (buffer[offset+0] <<  0) |
	       (buffer[offset+1] <<  8) |
	       (buffer[offset+2] << 16) |
	       (buffer[offset+3] << 24);
}

void write_u16_be(std::vector<u8>& buffer, size_t offset, u16 value) {
	buffer[offset+0] = value >> 8 & 0xff;
	buffer[offset+1] = value >> 0 & 0xff;
}

void write_u32_be(std::vector<u8>& buffer, size_t offset, u32 value) {
	buffer[offset+0] = value >> 24 & 0xff;
	buffer[offset+1] = value >> 16 & 0xff;
	buffer[offset+2] = value >>  8 & 0xff;
	buffer[offset+3] = value >>  0 & 0xff;
}

bool is_equal(std::string str1, std::string str2) {
	return std::strcmp(str1.c_str(), str2.c_str()) == 0;
}

result_t read_file(const fs::path& filename, std::vector<u8>& contents) {
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
	contents.insert(contents.begin(),
				    std::istream_iterator<u8>(fstream),
				    std::istream_iterator<u8>());

	fstream.close();

	return 0;
}

void write_file(const fs::path& filename, const std::vector<u8>& contents) {
	std::ofstream fstream(filename, std::ios::out | std::ios::binary);
	fstream.write(reinterpret_cast<const char*>(contents.data()), contents.size());
}

}


