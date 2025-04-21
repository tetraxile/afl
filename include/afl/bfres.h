#ifndef BFRES_H
#define BFRES_H

#include <vector>

#include "afl/util.h"

class BFRES {
public:
	BFRES(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	result_t read();
	result_t readHeader(const u8* offset);

private:
	const std::vector<u8>& mContents;
	util::ByteOrder mByteOrder;
};

#endif
