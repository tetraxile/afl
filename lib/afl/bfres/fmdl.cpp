#include "afl/bfres.h"

namespace bfres {

result_t FMDL::read(const u8* offset) {
	result_t r;
	r = readHeader(offset, "FMDL");
	if (r) return r;

	u64 nameOffset = reader::readU64(offset + 0x10, mByteOrder);
	u64 pathNameOffset = reader::readU64(offset + 0x18, mByteOrder);
	u64 sklOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 vtxBufOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 shpArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 shpDictOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 matArrayOffset = reader::readU64(offset + 0x40, mByteOrder);
	u64 matDictOffset = reader::readU64(offset + 0x48, mByteOrder);
	u64 userDataOffset = reader::readU64(offset + 0x50, mByteOrder);
	u16 vtxBufCount = reader::readU16(offset + 0x68, mByteOrder);
	u16 shpCount = reader::readU16(offset + 0x6a, mByteOrder);
	u16 matCount = reader::readU16(offset + 0x6c, mByteOrder);
	u16 userDataCount = reader::readU16(offset + 0x6e, mByteOrder);
	mTotalVtxCount = reader::readU32(offset + 0x70, mByteOrder);

	u16 nameLen = reader::readU16(mBase + nameOffset, mByteOrder);
	mName = reader::readString(mBase + nameOffset + 2, nameLen);

	u16 pathNameLen = reader::readU16(mBase + pathNameOffset, mByteOrder);
	mPathName = reader::readString(mBase + pathNameOffset + 2, pathNameLen);

	printf("\tname: %s\n", mName.c_str());
	printf("\n");

	if (sklOffset) {
		printf("\tskeleton offset: %lx\n", sklOffset);
		printf("\n");
	}

	if (vtxBufCount) {
		printf("\tvertex buffers: %d\n", vtxBufCount);
		printf("\tentries:\n");
		for (u16 i = 0; i < vtxBufCount; i++) {
			FVTX* vtxBuffer = new FVTX(mFile, mBase, mByteOrder);
			r = vtxBuffer->read(mBase + vtxBufOffset + i * FVTX::cSize);
			if (r) return r;
			mVtxBuffers.push_back(vtxBuffer);
		}
		printf("\n");
	}

	if (shpCount) {
		printf("\tshapes: %d\n", shpCount);
		printf("\tentries:\n");
		mShapes = new Dict<FSHP>(mFile, mBase, mByteOrder);
		r = mShapes->read(shpDictOffset, shpArrayOffset);
		if (r) return r;
		printf("\n");
	}

	if (matCount) {
		printf("\tmaterials:\n");
		printf("\t\tarray offset: %lx\n", matArrayOffset);
		printf("\t\tdict offset: %lx\n", matDictOffset);
		printf("\t\tcount: %d\n", matCount);
		printf("\tentries:\n");
		for (u16 i = 0; i < matCount; i++) {
			printf("\t\tmaterial:\n");
		}
		printf("\n");
	}

	return 0;
}

} // namespace bfres
