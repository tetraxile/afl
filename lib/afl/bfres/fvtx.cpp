#include "afl/bfres.h"

namespace bfres {

result_t FVTX::read(const u8* offset) {
	result_t r;
	r = readHeader(offset, "FVTX");
	if (r) return r;

	u64 attrArrayOffset = reader::readU64(offset + 0x10, mByteOrder);
	u64 attrDictOffset = reader::readU64(offset + 0x18, mByteOrder);
	mMemoryPoolOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 runtimeVtxBufArrayOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 userBufferArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 bufferSizeOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 strideOffset = reader::readU64(offset + 0x40, mByteOrder);
	// u64 bufferArrayOffset = reader::readU64(offset + 0x48, mByteOrder);
	u32 bufferOffset = reader::readU32(offset + 0x50, mByteOrder);
	u8 attrCount = reader::readU8(offset + 0x54);
	u8 bufferCount = reader::readU8(offset + 0x55);
	mIndex = reader::readU16(offset + 0x56, mByteOrder);
	mVertexCount = reader::readU32(offset + 0x58, mByteOrder);
	mSkinWeightInfluence = reader::readU32(offset + 0x5c, mByteOrder);

	printf("\t\tbuffers:\n");
	const u8* bufOffset = mBase + mFile->getGPUBufferOffset() + bufferOffset;
	for (s32 i = 0; i < bufferCount; i++) {
		u32 bufferSize = reader::readU32(mBase + bufferSizeOffset + i * 0x10, mByteOrder);
		u32 stride = reader::readU32(mBase + strideOffset + i * 0x10, mByteOrder);
		u32 elemCount = bufferSize / stride;

		printf("\t\t\toffset: %lx\n", bufOffset - mBase);
		printf("\t\t\tcount: %d\n", elemCount);
		printf("\t\t\tstride: %d\n", stride);
		printf("\n");

		mBuffers.push_back(new VertexBuffer(bufOffset, elemCount, stride));
		bufOffset += bufferSize;
	}

	if (attrCount) {
		printf("\t\tattributes:\n");
		mAttrs = new Dict<VertexAttribute>(mFile, mBase, mByteOrder);
		mAttrs->read(attrDictOffset, attrArrayOffset);
	}

	return 0;
}

result_t VertexAttribute::read(const u8* offset) {
	u64 nameOffset = reader::readU64(offset, mByteOrder);
	mFormat = AttributeFormat(reader::readU32(offset + 0x8, mByteOrder));
	mBufferOffset = reader::readU16(offset + 0xc, mByteOrder);
	mBufferIdx = reader::readU8(offset + 0xe);
	mIsDynamic = reader::readU8(offset + 0xf);

	u16 nameLen = reader::readU16(mBase + nameOffset, mByteOrder);
	mName = reader::readString(mBase + nameOffset + 2, nameLen);

	printf("\t\t\tname: %s\n", mName.c_str());
	printf("\t\t\tformat: %x\n", (u32)mFormat);
	printf("\t\t\tbuffer offset: %04x\n", mBufferOffset);
	printf("\t\t\tbuffer index: %d\n", mBufferIdx);
	printf("\t\t\tis dynamic?: %s\n", mIsDynamic ? "true" : "false");
	printf("\n");

	return 0;
}

} // namespace bfres
