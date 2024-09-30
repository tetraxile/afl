#include "sarc.h"

result_t SARC::read() {
	result_t r;
	r = read_header();
	if (r) return r;

	r = read_sfat();
	if (r) return r;

	r = read_sfnt();
	if (r) return r;

	return 0;
}

result_t SARC::read_header() {
	result_t r;
	r = mReader.check_signature("SARC", 4);
	if (r) return r;

	u16 headerSize = mReader.read_u16();
	r = mReader.read_byte_order();
	if (r) return r;

	u32 fileSize = mReader.read_u32();
	mHeader.mDataOffset = mReader.read_u32();
	mHeader.mVersion = mReader.read_u16();
	mReader.seek_rel(2);

	return 0;
}

result_t SARC::read_sfat() {
	result_t r;
	r = mReader.check_signature("SFAT", 4);
	if (r) return r;
	u16 headerSize = mReader.read_u16();
	u16 nodeCount = mReader.read_u16();
	u32 hashKey = mReader.read_u32();

	mFiles.reserve(nodeCount);
	for (s32 i = 0; i < nodeCount; i++) {
		File file;
		file.mHash = mReader.read_u32();
		file.mAttrs = mReader.read_u32();
		file.mStartOffset = mReader.read_u32();
		file.mEndOffset = mReader.read_u32();
		mFiles.push_back(file);
	}

	return 0;
}

result_t SARC::read_sfnt() {
	result_t r;
	r = mReader.check_signature("SFNT", 4);
	if (r) return r;
	u16 headerSize = mReader.read_u16();
	mReader.seek_rel(2);

	u32 nameTableOffset = mReader.pos();
	for (File& file : mFiles) {
		if (file.mAttrs & (1 << 24)) {
			u16 nameOffset = (file.mAttrs & 0xffff) * 4;
			mReader.seek(nameTableOffset + nameOffset);
			file.mName = mReader.read_string();
		}
	}

	return 0;
}

result_t SARC::save(const char* outDir) {
	result_t r;

	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (File& file : mFiles) {
		u32 size = file.mEndOffset - file.mStartOffset;
		mReader.seek(mHeader.mDataOffset + file.mStartOffset);
		std::vector<u8> contents = mReader.read_bytes(size);
		util::write_file(basePath / file.mName, contents);
	}

	return 0;
}
