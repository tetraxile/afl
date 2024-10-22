#include "bntx.h"

#include <cstdio>

result_t BNTX::read() {
	result_t r;
	r = read_header(&mContents[0]);
	if (r) return r;

	return 0;
}

result_t BNTX::read_header(const u8* offset) {
	result_t r;
	r = reader::check_signature(offset, "BNTX\0\0\0\0", 8);
	if (r) return r;

	u32 version = reader::read_u32(offset + 8, util::ByteOrder::Big);
	r = reader::read_byte_order(&mByteOrder, offset + 0xc, 0xFEFF);
	if (r) return r;
	u8 alignment = reader::read_u8(offset + 0xe, mByteOrder);
	u8 targetAddrSize = reader::read_u8(offset + 0xf, mByteOrder);
	u32 filenameOffset = reader::read_u32(offset + 0x10, mByteOrder);
	u16 isRelocated = reader::read_u16(offset + 0x14, mByteOrder);
	u16 firstBlockOffset = reader::read_u16(offset + 0x16, mByteOrder);
	u32 relocTableOffset = reader::read_u32(offset + 0x18, mByteOrder);
	u32 fileSize = reader::read_u32(offset + 0x1c, mByteOrder);

	return 0;
}

