#ifndef SARC_H
#define SARC_H

#include "util.h"

class SARC {
public:
	struct Header {
		util::ByteOrder mByteOrder;
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

	SARC(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	result_t read();
	result_t read_header(const u8* offset);
	result_t read_sfat(const u8* offset);
	result_t read_sfnt(const u8* offset);
	const std::vector<std::string> get_filenames();
	result_t save_file(const char* outDir, const char* filename);
	result_t save_all(const char* outDir);
	result_t get_file_data(std::vector<u8>& out, const char* filename);

private:
	const std::vector<u8>& mContents;
	Header mHeader;
	std::vector<File> mFiles;
};

#endif
