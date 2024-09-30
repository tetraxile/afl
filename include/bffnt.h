#ifndef BFFNT_H
#define BFFNT_H

#include <vector>

#include "types.h"
#include "util.h"

class BFFNT {
public:
	struct FontWidth {
		u8 mLeftWidth;
		u8 mGlyphWidth;
		u8 mCharWidth;
	};

	struct FINF {
		u8 mFontType;
		u8 mHeight;
		u8 mWidth;
		u8 mAscent;
		u16 mLineFeed;
		u16 mAlternateCharIndex;
		FontWidth mDefaultCharWidth;
		u8 mEncoding;
		u32 mTGLPOffset;
		u32 mCWDHOffset;
		u32 mCMAPOffset;
	};

	struct TGLP {
		u8 mCellWidth;
		u8 mCellHeight;
		u8 mTexCount;
		u8 mMaxCharWidth;
		u32 mPerTexSize;
		u16 mBaselinePos;
		u16 mTexFormat;
		u16 mCellsPerRow;
		u16 mCellsPerCol;
		u16 mImageWidth;
		u16 mImageHeight;
		u32 mImageDataOffset;
	};

	struct CWDH {
		u16 mFirstEntryIdx;
		u16 mLastEntryIdx;
		u32 mNextCWDHOffset;
		std::vector<FontWidth> mWidths;
	};

	struct CMAP {
		u32 mRangeBegin;
		u32 mRangeEnd;
		u16 mMapMethod;
		u32 mNextCMAPOffset;
		std::vector<std::pair<u32, u32>> mMap;
	};

	BFFNT(const std::vector<u8>& fileContents) : mReader(fileContents){}

	result_t read();
	result_t read_header();
	result_t read_finf();
	result_t read_tglp(u32 offset);
	result_t read_cwdh(CWDH* cwdh, u32 offset);
	result_t read_cmap(CMAP* cmap, u32 offset);

private:
	util::BinaryReader mReader;
	FINF mFontInfo;
	TGLP mTexGlyph;
	CWDH mCharWidth;
	CMAP mCharMap;
};

#endif
