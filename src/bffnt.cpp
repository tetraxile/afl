#include "bffnt.h"

#include <cassert>
#include <cstdio>

#include "util.h"

result_t BFFNT::read_header() {
	result_t r;
	r = mReader.check_signature("FFNT", 4);
	if (r) return r;

	mReader.read_byte_order();
	u16 headerSize = mReader.read_u16();
	u32 version = mReader.read_u32();
	u32 fileSize = mReader.read_u32();
	u16 sectionCount = mReader.read_u16();
	mReader.seek_rel(2);

	return 0;
}

result_t BFFNT::read_finf() {
	result_t r;
	r = mReader.check_signature("FINF", 4);
	if (r) return r;

	u32 blockSize = mReader.read_u32();
	mFontInfo.mFontType = mReader.read_u8();
	mFontInfo.mHeight = mReader.read_u8();
	mFontInfo.mWidth = mReader.read_u8();
	mFontInfo.mAscent = mReader.read_u8();
	mFontInfo.mLineFeed = mReader.read_u16();
	mFontInfo.mAlternateCharIndex = mReader.read_u16();
	mFontInfo.mDefaultCharWidth.mLeftWidth = mReader.read_u8();
	mFontInfo.mDefaultCharWidth.mGlyphWidth = mReader.read_u8();
	mFontInfo.mDefaultCharWidth.mCharWidth = mReader.read_u8();
	mFontInfo.mEncoding = mReader.read_u8();
	mFontInfo.mTGLPOffset = mReader.read_u32();
	mFontInfo.mCWDHOffset = mReader.read_u32();
	mFontInfo.mCMAPOffset = mReader.read_u32();

	// printf("block size: %x\n", blockSize);
	// printf("font type: %x\n", fontType);
	// printf("height: %x\n", height);
	// printf("width: %x\n", width);
	// printf("ascent: %x\n", ascent);
	// printf("line feed: %x\n", lineFeed);
	// printf("alt char index: %x\n", alternateCharIndex);
	// printf("widths: %x %x %x\n", leftWidth, glyphWidth, charWidth);
	// printf("encoding: %x\n", charEncoding);
	// printf("TGLP: %x\n", tglpOffset);
	// printf("CWDH: %x\n", cwdhOffset);
	// printf("CMAP: %x\n", cmapOffset);
	
	return 0;
}

result_t BFFNT::read_tglp(u32 offset) {
	result_t r;
	mReader.seek(offset - 8);
	r = mReader.check_signature("TGLP", 4);
	if (r) return r;
	
	u32 blockSize = mReader.read_u32();
	mTexGlyph.mCellWidth = mReader.read_u8();
	mTexGlyph.mCellHeight = mReader.read_u8();
	mTexGlyph.mTexCount = mReader.read_u8();
	mTexGlyph.mMaxCharWidth = mReader.read_u8();
	mTexGlyph.mPerTexSize = mReader.read_u32();
	mTexGlyph.mBaselinePos = mReader.read_u16();
	mTexGlyph.mTexFormat = mReader.read_u16();
	mTexGlyph.mCellsPerRow = mReader.read_u16();
	mTexGlyph.mCellsPerCol = mReader.read_u16();
	mTexGlyph.mImageWidth = mReader.read_u16();
	mTexGlyph.mImageHeight = mReader.read_u16();
	mTexGlyph.mImageDataOffset = mReader.read_u32();

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

result_t BFFNT::read_cwdh(BFFNT::CWDH* cwdh, u32 offset) {
	assert(cwdh != nullptr);

	result_t r;
	mReader.seek(offset - 8);
	r = mReader.check_signature("CWDH", 4);
	if (r) return r;

	u32 blockSize = mReader.read_u32();
	cwdh->mFirstEntryIdx = mReader.read_u16();
	cwdh->mLastEntryIdx = mReader.read_u16();
	cwdh->mNextCWDHOffset = mReader.read_u32();

	std::vector<FontWidth> entries;
	for (u16 i = cwdh->mFirstEntryIdx; i < cwdh->mLastEntryIdx; i++) {
		FontWidth width;
		width.mLeftWidth = mReader.read_u8();
		width.mGlyphWidth = mReader.read_u8();
		width.mCharWidth = mReader.read_u8();
		entries.push_back(width);
	}	

	// printf("block size: %x\n", blockSize);
	// printf("first entry idx: %x\n", firstEntryIdx);
	// printf("last entry idx: %x\n", lastEntryIdx);
	// printf("next CWDH offset: %x\n", nextCWDHOffset);
	//
	// for (auto& width : entries) {
	// 	printf("\tleft: %x, glyph: %x, char: %x\n", width.mLeftWidth, width.mGlyphWidth, width.mCharWidth);
	// }
	
	return 0;
}

result_t BFFNT::read_cmap(BFFNT::CMAP* cmap, u32 offset) {
	assert(cmap != nullptr);

	result_t r;
	mReader.seek(offset - 8);
	r = mReader.check_signature("CMAP", 4);
	if (r) return r;

	u32 blockSize = mReader.read_u32();
	cmap->mRangeBegin = mReader.read_u32();
	cmap->mRangeEnd = mReader.read_u32();
	cmap->mMapMethod = mReader.read_u16();
	mReader.seek_rel(2);
	cmap->mNextCMAPOffset = mReader.read_u32();


	// printf("block size: %x\n", blockSize);
	// printf("range begin: %x\n", rangeBegin);
	// printf("range end: %x\n", rangeEnd);
	// printf("map method: %x\n", mapMethod);
	// printf("next CMAP offset: %x\n", nextCMAPOffset);


	if (cmap->mMapMethod == 0) { // direct
		u16 charCode = mReader.read_u16();
		// printf("\t%x\n", charCode);
	} else if (cmap->mMapMethod == 1) { // table
		std::vector<u16> range;
		for (s32 i = cmap->mRangeBegin; i < cmap->mRangeEnd; i++) {
			u16 charCode = mReader.read_u16();
			range.push_back(charCode);
		}
		for (auto code : range) {
			// printf("\t%x\n", code);
		}
	} else if (cmap->mMapMethod == 2) { // scan
		u16 halfCount = mReader.read_u16();
		mReader.seek_rel(2);
		for (s32 i = 0; i < halfCount; i++) {
			u32 key = mReader.read_u32();
			u32 code = mReader.read_u32();
			// printf("\t%x -> %x\n", key, code);
		}
	}

	return 0;
}

result_t BFFNT::read() {
	result_t r;
	// file header
	r = read_header();
	if (r) return r;
	
	// font info
	r = read_finf();
	if (r) return r;

	// texture glyph
	r = read_tglp(mFontInfo.mTGLPOffset);
	if (r) return r;

	mReader.seek(mTexGlyph.mImageDataOffset);
	std::vector<u8> imageData = mReader.read_bytes(mTexGlyph.mPerTexSize);

	util::write_file("out.bntx", imageData);

	// character width table
	u32 nextOffset = mFontInfo.mCWDHOffset;
	while (nextOffset != 0) {
		CWDH cwdh;
		r = read_cwdh(&cwdh, nextOffset);
		if (r) return r;
		nextOffset = cwdh.mNextCWDHOffset;
	}

	// character map
	nextOffset = mFontInfo.mCMAPOffset;
	while (nextOffset != 0) {
		CMAP cmap;
		r = read_cmap(&cmap, nextOffset);
		if (r) return r;
		nextOffset = cmap.mNextCMAPOffset;
	}

	return 0;
}

