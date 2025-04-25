#include "afl/sarc.h"

#include <cassert>

result_t SARC::read() {
	result_t r;
	r = readHeader(&mContents[0]);
	if (r) return r;

	r = readSFAT(&mContents[0x14]);
	if (r) return r;

	r = readSFNT(&mContents[0x14 + 0xc + 0x10 * mFiles.size()]);
	if (r) return r;

	return 0;
}

result_t SARC::readHeader(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "SARC", 4);
	if (r) return r;

	u16 headerSize = reader::readU16(offset + 4, util::ByteOrder::Little);
	r = reader::readByteOrder(&mHeader.mByteOrder, offset + 6, 0xFEFF);
	if (r) return r;

	mHeader.mFileSize = reader::readU32(offset + 8, mHeader.mByteOrder);
	mHeader.mDataOffset = reader::readU32(offset + 0xc, mHeader.mByteOrder);
	mHeader.mVersion = reader::readU16(offset + 0x10, mHeader.mByteOrder);
	assert(mHeader.mVersion == 0x100);
	// 2 padding bytes

	return 0;
}

result_t SARC::readSFAT(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "SFAT", 4);
	if (r) return r;
	u16 headerSize = reader::readU16(offset + 4, mHeader.mByteOrder);
	assert(headerSize == 0xc);
	u16 nodeCount = reader::readU16(offset + 6, mHeader.mByteOrder);
	u32 hashKey = reader::readU32(offset + 8, mHeader.mByteOrder);

	mFiles.reserve(nodeCount);
	for (s32 i = 0; i < nodeCount; i++) {
		const u8* fileOffset = offset + 0xc + 0x10 * i;
		File file;
		file.mHash = reader::readU32(fileOffset, mHeader.mByteOrder);
		file.mAttrs = reader::readU32(fileOffset + 4, mHeader.mByteOrder);
		file.mStartOffset = reader::readU32(fileOffset + 8, mHeader.mByteOrder);
		file.mEndOffset = reader::readU32(fileOffset + 0xc, mHeader.mByteOrder);
		mFiles.push_back(file);
	}

	return 0;
}

result_t SARC::readSFNT(const u8* offset) {
	result_t r = reader::checkSignature(offset, "SFNT", 4);
	if (r) return r;
	u16 headerSize = reader::readU16(offset + 4, mHeader.mByteOrder);
	assert(headerSize == 0x8);

	const u8* nameTableOffset = offset + 8;
	for (File& file : mFiles) {
		if (file.mAttrs & (1 << 24)) {
			u16 nameOffset = (file.mAttrs & 0xffff) * 4;
			file.mName = reader::readString(nameTableOffset + nameOffset);
		}
	}

	return 0;
}

const std::unordered_set<std::string> SARC::getFilenames() {
	std::unordered_set<std::string> filenames;
	for (const File& file : mFiles)
		filenames.insert(file.mName);

	return filenames;
}

result_t SARC::saveFile(const std::string& outDir, const std::string& filename) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::readBytes(offset, size);
		util::writeFile(basePath / file.mName, contents);
		return 0;
	}

	return util::Error::FileNotFound;
}

result_t SARC::saveAll(const std::string& outDir) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::readBytes(offset, size);
		util::writeFile(basePath / file.mName, contents);
	}

	return 0;
}

result_t SARC::getFileData(std::vector<u8>& out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		out = reader::readBytes(offset, size);
		return 0;
	}

	return util::Error::FileNotFound;
}

result_t SARC::getFileSize(u32* out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		*out = file.mEndOffset - file.mStartOffset;
		return 0;
	}

	return util::Error::FileNotFound;
}
