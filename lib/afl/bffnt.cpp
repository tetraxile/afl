#include "afl/bffnt.h"

#include <cassert>
#include <cstdio>

#include "afl/util.h"

result_t BFFNT::readHeader(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "FFNT", 4);
	if (r) return r;

	r = reader::readByteOrder(&mByteOrder, offset + 4, 0xFEFF);
	if (r) return r;

	u16 headerSize = reader::readU16(offset + 6, mByteOrder);
	u32 version = reader::readU32(offset + 8, mByteOrder);
	u32 fileSize = reader::readU32(offset + 0xc, mByteOrder);
	u16 sectionCount = reader::readU16(offset + 0x10, mByteOrder);

	return 0;
}

result_t BFFNT::readFINF(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "FINF", 4);
	if (r) return r;

	u32 blockSize = reader::readU32(offset + 4, mByteOrder);
	const u8* blockOffset = offset + 8;
	mFontInfo.mFontType = reader::readU8(blockOffset);
	mFontInfo.mHeight = reader::readU8(blockOffset + 1);
	mFontInfo.mWidth = reader::readU8(blockOffset + 2);
	mFontInfo.mAscent = reader::readU8(blockOffset + 3);
	mFontInfo.mLineFeed = reader::readU16(blockOffset + 4, mByteOrder);
	mFontInfo.mAlternateCharIndex = reader::readU16(blockOffset + 6, mByteOrder);
	mFontInfo.mDefaultCharWidth.mLeftWidth = reader::readU8(blockOffset + 8);
	mFontInfo.mDefaultCharWidth.mGlyphWidth = reader::readU8(blockOffset + 9);
	mFontInfo.mDefaultCharWidth.mCharWidth = reader::readU8(blockOffset + 0xa);
	mFontInfo.mEncoding = reader::readU8(blockOffset + 0xb);
	mFontInfo.mTGLPOffset = reader::readU32(blockOffset + 0xc, mByteOrder);
	mFontInfo.mCWDHOffset = reader::readU32(blockOffset + 0x10, mByteOrder);
	mFontInfo.mCMAPOffset = reader::readU32(blockOffset + 0x14, mByteOrder);

	// printf("block size: %x\n", blockSize);
	// printf("font type: %x\n", mFontInfo.mFontType);
	// printf("height: %x\n", mFontInfo.mHeight);
	// printf("width: %x\n", mFontInfo.mWidth);
	// printf("ascent: %x\n", mFontInfo.mAscent);
	// printf("line feed: %x\n", mFontInfo.mLineFeed);
	// printf("alt char index: %x\n", mFontInfo.mAlternateCharIndex);
	// printf("widths: %x %x %x\n", mFontInfo.mDefaultCharWidth.mLeftWidth,
	// mFontInfo.mDefaultCharWidth.mGlyphWidth, mFontInfo.mDefaultCharWidth.mCharWidth);
	// printf("encoding: %x\n", mFontInfo.mEncoding);
	// printf("TGLP: %x\n", mFontInfo.mTGLPOffset);
	// printf("CWDH: %x\n", mFontInfo.mCWDHOffset);
	// printf("CMAP: %x\n", mFontInfo.mCMAPOffset);

	return 0;
}

result_t BFFNT::readTGLP(const u8* offset) {
	result_t r;
	r = reader::checkSignature(offset, "TGLP", 4);
	if (r) return r;

	u32 blockSize = reader::readU32(offset + 4, mByteOrder);
	const u8* blockOffset = offset + 8;
	mTexGlyph.mCellWidth = reader::readU8(blockOffset);
	mTexGlyph.mCellHeight = reader::readU8(blockOffset + 1);
	mTexGlyph.mTexCount = reader::readU8(blockOffset + 2);
	mTexGlyph.mMaxCharWidth = reader::readU8(blockOffset + 3);
	mTexGlyph.mPerTexSize = reader::readU32(blockOffset + 4, mByteOrder);
	mTexGlyph.mBaselinePos = reader::readU16(blockOffset + 8, mByteOrder);
	mTexGlyph.mTexFormat = reader::readU16(blockOffset + 0xa, mByteOrder);
	mTexGlyph.mCellsPerRow = reader::readU16(blockOffset + 0xc, mByteOrder);
	mTexGlyph.mCellsPerCol = reader::readU16(blockOffset + 0xe, mByteOrder);
	mTexGlyph.mImageWidth = reader::readU16(blockOffset + 0x10, mByteOrder);
	mTexGlyph.mImageHeight = reader::readU16(blockOffset + 0x12, mByteOrder);
	mTexGlyph.mImageDataOffset = reader::readU32(blockOffset + 0x14, mByteOrder);

	// printf("block size: %x\n", blockSize);
	// printf("cell dims: %x x %x\n", mTexGlyph.mCellWidth, mTexGlyph.mCellHeight);
	// printf("tex count: %x\n", mTexGlyph.mTexCount);
	// printf("max char width: %x\n", mTexGlyph.mMaxCharWidth);
	// printf("per tex size: %x\n", mTexGlyph.mPerTexSize);
	// printf("baseline pos: %x\n", mTexGlyph.mBaselinePos);
	// printf("tex format: %x\n", mTexGlyph.mTexFormat);
	// printf("cells dims: %x x %x\n", mTexGlyph.mCellsPerRow, mTexGlyph.mCellsPerCol);
	// printf("image dims: %x x %x\n", mTexGlyph.mImageWidth, mTexGlyph.mImageHeight);
	// printf("image data offset: %x\n", mTexGlyph.mImageDataOffset);

	return 0;
}

result_t BFFNT::readCWDH(BFFNT::CWDH* cwdh, const u8* offset) {
	assert(cwdh != nullptr);

	result_t r;
	r = reader::checkSignature(offset, "CWDH", 4);
	if (r) return r;

	u32 blockSize = reader::readU32(offset + 4, mByteOrder);
	const u8* blockOffset = offset + 8;
	cwdh->mFirstEntryIdx = reader::readU16(blockOffset, mByteOrder);
	cwdh->mLastEntryIdx = reader::readU16(blockOffset + 2, mByteOrder);
	cwdh->mNextCWDHOffset = reader::readU32(blockOffset + 4, mByteOrder);

	std::vector<FontWidth> entries;
	for (u16 i = cwdh->mFirstEntryIdx; i < cwdh->mLastEntryIdx; i++) {
		FontWidth width;
		const u8* widthOffset = blockOffset + 8 + 3 * i;
		width.mLeftWidth = reader::readU8(widthOffset);
		width.mGlyphWidth = reader::readU8(widthOffset + 1);
		width.mCharWidth = reader::readU8(widthOffset + 2);
		entries.push_back(width);
	}

	// printf("block size: %x\n", blockSize);
	// printf("first entry idx: %x\n", cwdh->mFirstEntryIdx);
	// printf("last entry idx: %x\n", cwdh->mLastEntryIdx);
	// printf("next CWDH offset: %x\n", cwdh->mNextCWDHOffset);
	//
	// for (auto& width : entries) {
	// 	printf("\tleft: %x, glyph: %x, char: %x\n", width.mLeftWidth, width.mGlyphWidth,
	// width.mCharWidth);
	// }

	return 0;
}

result_t BFFNT::readCMAP(BFFNT::CMAP* cmap, const u8* offset) {
	assert(cmap != nullptr);

	result_t r;
	r = reader::checkSignature(offset, "CMAP", 4);
	if (r) return r;

	u32 blockSize = reader::readU32(offset + 4, mByteOrder);
	const u8* blockOffset = offset + 8;
	cmap->mRangeBegin = reader::readU32(blockOffset, mByteOrder);
	cmap->mRangeEnd = reader::readU32(blockOffset + 4, mByteOrder);
	cmap->mMapMethod = reader::readU16(blockOffset + 8, mByteOrder);
	// 2 bytes of padding
	cmap->mNextCMAPOffset = reader::readU32(blockOffset + 0xc, mByteOrder);

	// printf("block size: %x\n", blockSize);
	// printf("range begin: %x\n", cmap->mRangeBegin);
	// printf("range end: %x\n", cmap->mRangeEnd);
	// printf("map method: %x\n", cmap->mMapMethod);
	// printf("next CMAP offset: %x\n", cmap->mNextCMAPOffset);

	const u8* mapOffset = blockOffset + 0x10;
	if (cmap->mMapMethod == 0) { // direct
		u16 charCode = reader::readU16(mapOffset, mByteOrder);
		// printf("\t%x\n", charCode);
	} else if (cmap->mMapMethod == 1) { // table
		std::vector<u16> range;
		for (u32 i = cmap->mRangeBegin; i < cmap->mRangeEnd; i++) {
			u16 charCode = reader::readU16(mapOffset + 2 * i, mByteOrder);
			range.push_back(charCode);
		}
		for (auto code : range) {
			// printf("\t%x\n", code);
		}
	} else if (cmap->mMapMethod == 2) { // scan
		u16 halfCount = reader::readU16(mapOffset, mByteOrder);
		for (s32 i = 0; i < halfCount; i++) {
			u32 key = reader::readU32(mapOffset + 4 + 8 * i, mByteOrder);
			u32 code = reader::readU32(mapOffset + 4 + 8 * i + 4, mByteOrder);
			// printf("\t%x -> %x\n", key, code);
		}
	}

	return 0;
}

result_t BFFNT::read() {
	result_t r;
	// file header
	r = readHeader(&mContents[0]);
	if (r) return r;

	// font info
	r = readFINF(&mContents[0x14]);
	if (r) return r;

	// texture glyph
	r = readTGLP(&mContents[0] + mFontInfo.mTGLPOffset - 8);
	if (r) return r;

	const u8* imageOffset = &mContents[0] + mTexGlyph.mImageDataOffset;
	std::vector<u8> imageData = reader::readBytes(imageOffset, mTexGlyph.mPerTexSize);

	util::writeFile("out.bntx", imageData);

	// character width table
	const u8* nextOffset = &mContents[0] + mFontInfo.mCWDHOffset;
	while (nextOffset - &mContents[0] != 0) {
		CWDH cwdh;
		r = readCWDH(&cwdh, nextOffset - 8);
		if (r) return r;
		nextOffset = &mContents[0] + cwdh.mNextCWDHOffset;
	}

	// character map
	nextOffset = &mContents[0] + mFontInfo.mCMAPOffset;
	while (nextOffset - &mContents[0] != 0) {
		CMAP cmap;
		r = readCMAP(&cmap, nextOffset - 8);
		if (r) return r;
		nextOffset = &mContents[0] + cmap.mNextCMAPOffset;
	}

	return 0;
}
