#include "afl/bntx.h"

#include <cstdio>

result_t BNTX::read() {
	result_t r;
	r = readHeader(&mContents[0]);
	if (r) return r;

	return 0;
}

result_t BNTX::readHeader(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "BNTX\0\0\0\0", 8);
	if (r) return r;

	u32 version = reader::readU32(offset + 8, util::ByteOrder::Big);
	r = reader::readByteOrder(&mByteOrder, offset + 0xc, 0xFEFF);
	if (r) return r;
	u8 alignment = reader::readU8(offset + 0xe);
	u8 targetAddrSize = reader::readU8(offset + 0xf);
	u32 filenameOffset = reader::readU32(offset + 0x10, mByteOrder);
	u16 isRelocated = reader::readU16(offset + 0x14, mByteOrder);
	u16 firstBlockOffset = reader::readU16(offset + 0x16, mByteOrder);
	u32 relocTableOffset = reader::readU32(offset + 0x18, mByteOrder);
	u32 fileSize = reader::readU32(offset + 0x1c, mByteOrder);

	return 0;
}
