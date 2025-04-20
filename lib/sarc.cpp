#include "sarc.h"
#include <cassert>

result_t SARC::read() {
	result_t r;
	r = read_header(&mContents[0]);
	if (r) return r;

	r = read_sfat(&mContents[0x14]);
	if (r) return r;

	r = read_sfnt(&mContents[0x14 + 0xc + 0x10 * mFiles.size()]);
	if (r) return r;

	return 0;
}

result_t SARC::read_header(const u8* offset) {
	result_t r;
	r = reader::check_signature(offset, "SARC", 4);
	if (r) return r;

	u16 headerSize = reader::read_u16(offset + 4, util::ByteOrder::Little);
	r = reader::read_byte_order(&mHeader.mByteOrder, offset + 6, 0xFEFF);
	if (r) return r;

	mHeader.mFileSize = reader::read_u32(offset + 8, mHeader.mByteOrder);
	mHeader.mDataOffset = reader::read_u32(offset + 0xc, mHeader.mByteOrder);
	mHeader.mVersion = reader::read_u16(offset + 0x10, mHeader.mByteOrder);
	assert(mHeader.mVersion == 0x100);
	// 2 padding bytes

	return 0;
}

result_t SARC::read_sfat(const u8* offset) {
	result_t r;
	r = reader::check_signature(offset, "SFAT", 4);
	if (r) return r;
	u16 headerSize = reader::read_u16(offset + 4, mHeader.mByteOrder);
	assert(headerSize == 0xc);
	u16 nodeCount = reader::read_u16(offset + 6, mHeader.mByteOrder);
	u32 hashKey = reader::read_u32(offset + 8, mHeader.mByteOrder);

	mFiles.reserve(nodeCount);
	for (s32 i = 0; i < nodeCount; i++) {
		const u8* fileOffset = offset + 0xc + 0x10 * i;
		File file;
		file.mHash = reader::read_u32(fileOffset, mHeader.mByteOrder);
		file.mAttrs = reader::read_u32(fileOffset + 4, mHeader.mByteOrder);
		file.mStartOffset = reader::read_u32(fileOffset + 8, mHeader.mByteOrder);
		file.mEndOffset = reader::read_u32(fileOffset + 0xc, mHeader.mByteOrder);
		mFiles.push_back(file);
	}

	return 0;
}

result_t SARC::read_sfnt(const u8* offset) {
	result_t r = reader::check_signature(offset, "SFNT", 4);
	if (r) return r;
	u16 headerSize = reader::read_u16(offset + 4, mHeader.mByteOrder);
	assert(headerSize == 0x8);

	const u8* nameTableOffset = offset + 8;
	for (File& file : mFiles) {
		if (file.mAttrs & (1 << 24)) {
			u16 nameOffset = (file.mAttrs & 0xffff) * 4;
			file.mName = reader::read_string(nameTableOffset + nameOffset);
		}
	}

	return 0;
}

const std::vector<std::string> SARC::get_filenames() {
	std::vector<std::string> mFilenames;
	for (const File& file : mFiles)
		mFilenames.push_back(file.mName);

	return mFilenames;
}

result_t SARC::save_file(const std::string& outDir, const std::string& filename) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		if (!util::is_equal(file.mName, filename)) continue;

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::read_bytes(offset, size);
		util::write_file(basePath / file.mName, contents);
		return 0;
	}

	return Error::FileNotFound;
}

result_t SARC::save_all(const std::string& outDir) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::read_bytes(offset, size);
		util::write_file(basePath / file.mName, contents);
	}

	return 0;
}

result_t SARC::get_file_data(std::vector<u8>& out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::is_equal(file.mName, filename)) continue;

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		out = reader::read_bytes(offset, size);
		return 0;
	}

	return Error::FileNotFound;
}

result_t SARC::get_file_size(u32* out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::is_equal(file.mName, filename)) continue;

		*out = file.mEndOffset - file.mStartOffset;
		return 0;
	}

	return Error::FileNotFound;
}
