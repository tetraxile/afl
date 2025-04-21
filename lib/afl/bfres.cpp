#include "afl/bfres.h"

#include <cstdio>

result_t BFRES::read() {
	result_t r;
	r = readHeader(&mContents[0]);
	if (r) return r;

	return 0;
}

result_t BFRES::readHeader(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "FRES    ", 8);
	if (r) return r;

	u32 version = reader::readU32(offset + 8, util::ByteOrder::Big);
	r = reader::readByteOrder(&mByteOrder, offset + 0xc, 0xFEFF);
	if (r) return r;

	u8 alignment         = reader::readU8(offset + 0xe);
	u8 targetAddrSize    = reader::readU8(offset + 0xf);
	u32 filenameOffset   = reader::readU32(offset + 0x10, mByteOrder);
	u16 isRelocated      = reader::readU16(offset + 0x14, mByteOrder);
	u16 firstBlockOffset = reader::readU16(offset + 0x16, mByteOrder);
	u32 relocTableOffset = reader::readU32(offset + 0x18, mByteOrder);
	u32 fileSize         = reader::readU32(offset + 0x1c, mByteOrder);

	u64 filenameOffsetPrefixed  = reader::readU64(offset + 0x20, mByteOrder);
	u64 modelArrayOffset        = reader::readU64(offset + 0x28, mByteOrder);
	u64 modelDictOffset         = reader::readU64(offset + 0x30, mByteOrder);
	u64 sklAnimArrayOffset      = reader::readU64(offset + 0x38, mByteOrder);
	u64 sklAnimDictOffset       = reader::readU64(offset + 0x40, mByteOrder);
	u64 matAnimArrayOffset      = reader::readU64(offset + 0x48, mByteOrder);
	u64 matAnimDictOffset       = reader::readU64(offset + 0x50, mByteOrder);
	u64 visAnimArrayOffset      = reader::readU64(offset + 0x58, mByteOrder);
	u64 visAnimDictOffset       = reader::readU64(offset + 0x60, mByteOrder);
	u64 shpAnimArrayOffset      = reader::readU64(offset + 0x68, mByteOrder);
	u64 shpAnimDictOffset       = reader::readU64(offset + 0x70, mByteOrder);
	u64 scnAnimArrayOffset      = reader::readU64(offset + 0x78, mByteOrder);
	u64 scnAnimDictOffset       = reader::readU64(offset + 0x80, mByteOrder);
	u64 memoryPoolOffset        = reader::readU64(offset + 0x88, mByteOrder);
	u64 memoryPoolInfo          = reader::readU64(offset + 0x90, mByteOrder);
	u64 embeddedFileArrayOffset = reader::readU64(offset + 0x98, mByteOrder);
	u64 embeddedFileDictOffset  = reader::readU64(offset + 0xa0, mByteOrder);
	u64 unk                     = reader::readU64(offset + 0xa8, mByteOrder);
	u64 stringPoolOffset        = reader::readU64(offset + 0xb0, mByteOrder);
	u32 stringPoolSize          = reader::readU32(offset + 0xb8, mByteOrder);
	u16 modelCount              = reader::readU16(offset + 0xbc, mByteOrder);
	u16 sklAnimCount            = reader::readU16(offset + 0xbe, mByteOrder);
	u16 matAnimCount            = reader::readU16(offset + 0xc0, mByteOrder);
	u16 visAnimCount            = reader::readU16(offset + 0xc2, mByteOrder);
	u16 shpAnimCount            = reader::readU16(offset + 0xc4, mByteOrder);
	u16 scnAnimCount            = reader::readU16(offset + 0xc6, mByteOrder);
	u16 embeddedFileCount       = reader::readU16(offset + 0xc8, mByteOrder);

	printf("model count: %d\n", modelCount);
	printf("skl anim count: %d\n", sklAnimCount);
	printf("mat anim count: %d\n", matAnimCount);
	printf("vis anim count: %d\n", visAnimCount);
	printf("shp anim count: %d\n", shpAnimCount);
	printf("scn anim count: %d\n", scnAnimCount);
	printf("embedded count: %d\n", embeddedFileCount);
	// for (u16 i = 0; i < modelCount; i++) {
	// 	printf("model:\n");
	// }

	// for (u16 i = 0; i < sklAnimCount; i++) {
	// 	printf("skeletal animation:\n");
	// }

	// for (u16 i = 0; i < matAnimCount; i++) {
	// 	printf("material animation:\n");
	// }

	// for (u16 i = 0; i < visAnimCount; i++) {
	// 	printf("bone visibility animation:\n");
	// }

	// for (u16 i = 0; i < shpAnimCount; i++) {
	// 	printf("shape animation:\n");
	// }

	// for (u16 i = 0; i < scnAnimCount; i++) {
	// 	printf("scene animation:\n");
	// }

	// for (u16 i = 0; i < embeddedFileCount; i++) {
	// 	const u8* entryOffset = offset + embeddedFileArrayOffset + i * 0x10;
	// 	u64 fileDataOffset = reader::readU64(entryOffset, mByteOrder);
	// 	u32 fileDataSize = reader::readU32(entryOffset + 0x8, mByteOrder);
	// 	printf("embedded file:\n");
	// 	printf("\toffset: %lx\n", fileDataOffset);
	// 	printf("\tsize: %x\n", fileDataSize);
	// }

	return 0;
}
