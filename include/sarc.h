#ifndef SARC_H
#define SARC_H

#include "util.h"

class SARC {
public:
	struct Header {
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

	SARC(const std::vector<u8>& fileContents) : mReader(fileContents){}

	result_t read();
	result_t read_header();
	result_t read_sfat();
	result_t read_sfnt();
	result_t save(const char* outDir);

private:
	util::BinaryReader mReader;
	Header mHeader;
	std::vector<File> mFiles;
};

#endif
