#pragma once

#include <set>

#include "afl/util.h"

namespace sarc {

class Reader {
public:
	struct Header {
		util::ByteOrder mByteOrder;
		u32 mFileSize;
		u32 mDataOffset;
		u16 mVersion;
	};

	struct File {
		u32 mHash;
		u32 mAttrs;
		u32 mStartOffset;
		u32 mEndOffset;
		std::string mName;
	};

	Reader(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	result_t init();
	result_t initHeader(const u8* offset);
	result_t readSFAT(const u8* offset);
	result_t readSFNT(const u8* offset);
	const std::set<std::string> getFilenames();
	result_t saveFile(const std::string& outDir, const std::string& filename);
	result_t saveAll(const std::string& outDir);
	result_t getFileData(std::vector<u8>& out, const std::string& filename);
	result_t getFileSize(u32* out, const std::string& filename);

private:
	const std::vector<u8>& mContents;
	Header mHeader;
	std::vector<File> mFiles;
};

} // namespace sarc
