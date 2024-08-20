#ifndef UTIL_H
#define UTIL_H

#include "types.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

namespace util {

bool isEqual(const char* str1, const char* str2) {
	return std::strcmp(str1, str2) == 0;
}

s32 readFile(const char* filename, std::vector<u8>& contents) {
	std::ifstream fstream(filename, std::ios::binary);

	if (fstream.eof() || fstream.fail()) {
		fprintf(stderr, "error: couldn't open file\n");
		return 1;
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

}

#endif
