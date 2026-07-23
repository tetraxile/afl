#include "mizuna/msbp/reader.h"

#include <hk/Result.h>
#include <hk/ValueOrResult.h>

#include "mizuna/msbp/results.h"
#include "mizuna/util.h"

namespace msbp {

Reader::~Reader() {
	for (auto [k, v] : mBlocks) {
		delete v;
	}
}

hk::Result Reader::read() {
	HK_TRY(readHeader(mContents.data()));

	u32 blockOffset = 0x20;
	for (u32 i = 0; i < mNumBlocks; i++) {
		BlockType type;
		u32 blockSize;
		HK_TRY(readBlockHeader(mContents.data() + blockOffset, &type, &blockSize));
		HK_TRY(readBlock(mContents.data() + blockOffset + 0x10, type, blockSize));
		blockOffset += 0x10 + util::roundUp(blockSize, 0x10);
	}

	if (mBlocks.contains(BlockType::CLR1) && mBlocks.contains(BlockType::CLB1)) {
		CLR1* clr1 = static_cast<CLR1*>(mBlocks[BlockType::CLR1]);
		CLB1* clb1 = static_cast<CLB1*>(mBlocks[BlockType::CLB1]);

		for (LabelBlock::Label label : clb1->labels) {
			if (label.itemIndex >= clr1->colours.size()) return ResultBufferOverrun();
			mColours[label.name] = clr1->colours[label.itemIndex];
		}
	}

	if (mBlocks.contains(BlockType::ATI2) && mBlocks.contains(BlockType::ALB1) &&
	    mBlocks.contains(BlockType::ALI2)) {
		fprintf(stderr, "error: attributes are unimplemented\n");
		return ResultUnimplemented();
	}

	if (mBlocks.contains(BlockType::SYL3) && mBlocks.contains(BlockType::SLB1)) {
		SYL3* syl3 = static_cast<SYL3*>(mBlocks[BlockType::SYL3]);
		SLB1* slb1 = static_cast<SLB1*>(mBlocks[BlockType::SLB1]);

		for (LabelBlock::Label label : slb1->labels) {
			if (label.itemIndex >= syl3->styles.size()) return ResultBufferOverrun();

			const SYL3::Style& style = syl3->styles[label.itemIndex];
			mStyles[label.name] = { style.regionWidth, style.lineNum, style.fontIndex,
				                    style.baseColourIndex };
		}
	}

	if (mBlocks.contains(BlockType::TGG2) && mBlocks.contains(BlockType::TAG2) &&
	    mBlocks.contains(BlockType::TGP2) && mBlocks.contains(BlockType::TGL2)) {
		TGG2* tgg2 = static_cast<TGG2*>(mBlocks[BlockType::TGG2]);
		TAG2* tag2 = static_cast<TAG2*>(mBlocks[BlockType::TAG2]);
		TGP2* tgp2 = static_cast<TGP2*>(mBlocks[BlockType::TGP2]);
		TGL2* tgl2 = static_cast<TGL2*>(mBlocks[BlockType::TGL2]);

		for (const TGG2::Group& group_ : tgg2->groups) {
			std::vector<Tag> tags;
			for (u16 tagIdx : group_.tagIndices) {
				if (tagIdx >= tag2->tags.size()) return ResultBufferOverrun();
				const TAG2::Tag& tag = tag2->tags[tagIdx];
				std::vector<TagParam> params;
				for (u16 paramIdx : tag.paramIndices) {
					if (paramIdx >= tgp2->params.size()) return ResultBufferOverrun();
					const TGP2::Param& param = tgp2->params[paramIdx];
					std::vector<std::string> strings;
					for (u16 stringIdx : param.stringIndices) {
						if (stringIdx >= tgl2->strings.size()) return ResultBufferOverrun();
						strings.push_back(tgl2->strings[stringIdx]);
					}
					params.push_back({ param.name, param.type, strings });
				}
				tags.push_back({ tag.name, params });
			}
			mTagGroups[group_.id] = { group_.name, tags };
		}
	}

	if (mBlocks.contains(BlockType::CTI1)) {
		CTI1* cti1 = static_cast<CTI1*>(mBlocks[BlockType::CTI1]);
		mFilenames = cti1->filenames;
	}

	return hk::ResultSuccess();
}

hk::Result Reader::readHeader(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "MsgPrjBn", 8));
	HK_TRY(reader::readByteOrder(&mByteOrder, offset + 8, 0xFEFF));

	u8 encodingVal = reader::readU8(offset + 0xc);
	if (encodingVal > 2) return ResultInvalidTextEncoding();
	mEncoding = static_cast<Encoding>(encodingVal);

	mVersion = reader::readU8(offset + 0xd);
	if (mVersion != 4) return ResultUnsupportedVersion();

	mNumBlocks = reader::readU16(offset + 0xe, mByteOrder);
	mFileSize = reader::readU32(offset + 0x12, mByteOrder);

	return hk::ResultSuccess();
}

hk::Result Reader::readBlockHeader(const u8* offset, BlockType* type, u32* size) {
	const std::string signature = reader::readString(offset, 4);
	*size = reader::readU32(offset + 4, mByteOrder);

	if (signature == "CLR1")
		*type = BlockType::CLR1;
	else if (signature == "CLB1")
		*type = BlockType::CLB1;
	else if (signature == "ATI2")
		*type = BlockType::ATI2;
	else if (signature == "ALB1")
		*type = BlockType::ALB1;
	else if (signature == "ALI2")
		*type = BlockType::ALI2;
	else if (signature == "TGG2")
		*type = BlockType::TGG2;
	else if (signature == "TAG2")
		*type = BlockType::TAG2;
	else if (signature == "TGP2")
		*type = BlockType::TGP2;
	else if (signature == "TGL2")
		*type = BlockType::TGL2;
	else if (signature == "SYL3")
		*type = BlockType::SYL3;
	else if (signature == "SLB1")
		*type = BlockType::SLB1;
	else if (signature == "CTI1")
		*type = BlockType::CTI1;
	else
		return ResultInvalidBlockType();

	return hk::ResultSuccess();
}

hk::Result Reader::readBlock(const u8* offset, BlockType type, u32 size) {
	if (mBlocks.contains(type)) return ResultDuplicateBlockType();

	Block* block = nullptr;

	switch (type) {
	case BlockType::CLR1: block = new CLR1; break;
	case BlockType::CLB1: block = new CLB1; break;
	case BlockType::ATI2: block = new ATI2; break;
	case BlockType::ALB1: block = new ALB1; break;
	case BlockType::ALI2: block = new ALI2; break;
	case BlockType::TGG2: block = new TGG2; break;
	case BlockType::TAG2: block = new TAG2; break;
	case BlockType::TGP2: block = new TGP2; break;
	case BlockType::TGL2: block = new TGL2; break;
	case BlockType::SYL3: block = new SYL3; break;
	case BlockType::SLB1: block = new SLB1; break;
	case BlockType::CTI1: block = new CTI1; break;
	}

	if (!block) {
		fprintf(stderr, "fatal: unreachable\n");
		return hk::ResultFailed();
	}

	HK_TRY(block->read(offset, size, mByteOrder));
	mBlocks[type] = block;

	return hk::ResultSuccess();
}

hk::Result Reader::LabelBlock::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numBuckets = reader::readU32(offset, byteOrder);
	if (numBuckets != 29) return ResultWrongHashBucketCount();

	const u8* labelsStart = offset + 4 + 8 * numBuckets;
	u32 totalLabelCount = 0;
	for (u32 i = 0; i < numBuckets; i++) {
		totalLabelCount += reader::readU32(offset + 4 + i * 8, byteOrder);
		u32 labelsOffset = reader::readU32(offset + 4 + i * 8 + 4, byteOrder);
	}

	const u8* labelOffset = labelsStart;
	for (u32 i = 0; i < totalLabelCount; i++) {
		u8 labelLen = reader::readU8(labelOffset);
		std::string label = reader::readString(labelOffset + 1, labelLen);
		u32 itemIndex = reader::readU32(labelOffset + 1 + labelLen, byteOrder);

		labels.push_back({ label, itemIndex });

		labelOffset += 1 + labelLen + 4;
	}

	return hk::ResultSuccess();
}

hk::Result Reader::CLR1::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numColours = reader::readU32(offset, byteOrder);
	if (size < 4 + 4 * numColours) return ResultBlockSizeOverrun();

	for (u32 i = 0; i < numColours; i++)
		colours.push_back(reader::readU32(offset + 4 + 4 * i, byteOrder));

	return hk::ResultSuccess();
}

hk::Result Reader::ATI2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numAttributes = reader::readU32(offset, byteOrder);
	if (size < 4 + 8 * numAttributes) return ResultBlockSizeOverrun();

	for (u32 i = 0; i < numAttributes; i++) {
		u8 type = reader::readU8(offset + 4 + i * 8);
		u16 listIdx = reader::readU16(offset + 4 + i * 8 + 2, byteOrder);
		u32 listOffset = reader::readU32(offset + 4 + i * 8 + 4, byteOrder);
		attributes.push_back({ type, listIdx, listOffset });
	}

	return hk::ResultSuccess();
}

hk::Result Reader::ALI2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numLists = reader::readU32(offset, byteOrder);

	for (u32 i = 0; i < numLists; i++) {
		u32 listOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);

		const u8* listStart = offset + listOffset;
		u32 listSize = reader::readU32(listStart, byteOrder);

		std::vector<std::string> items;
		for (u32 j = 0; j < listSize; j++) {
			u32 itemNameOffset = reader::readU32(listStart + 4 + 4 * j, byteOrder);
			std::string itemName = reader::readString(offset + itemNameOffset);
			items.push_back(itemName);
		}

		lists.push_back(items);
	}

	return hk::ResultSuccess();
}

hk::Result Reader::TGG2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u16 numGroups = reader::readU16(offset, byteOrder);

	for (u16 i = 0; i < numGroups; i++) {
		u32 groupOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);

		const u8* groupStart = offset + groupOffset;
		u16 groupId = reader::readU16(groupStart, byteOrder);
		u16 numTags = reader::readU16(groupStart + 2, byteOrder);

		std::vector<u16> tagIndices;
		for (u16 j = 0; j < numTags; j++)
			tagIndices.push_back(reader::readU16(groupStart + 4 + 2 * j, byteOrder));

		std::string groupName = reader::readString(groupStart + 4 + 2 * numTags);

		groups.push_back({ groupId, groupName, tagIndices });
	}

	return hk::ResultSuccess();
}

hk::Result Reader::TAG2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u16 numTags = reader::readU16(offset, byteOrder);

	for (u16 i = 0; i < numTags; i++) {
		u32 tagOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);

		const u8* tagStart = offset + tagOffset;
		u16 numParams = reader::readU16(tagStart, byteOrder);

		std::vector<u16> paramIndices;
		for (u16 j = 0; j < numParams; j++)
			paramIndices.push_back(reader::readU16(tagStart + 2 + 2 * j, byteOrder));

		std::string tagName = reader::readString(tagStart + 2 + 2 * numParams);

		tags.push_back({ tagName, paramIndices });
	}

	return hk::ResultSuccess();
}

hk::Result Reader::TGP2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u16 numParams = reader::readU16(offset, byteOrder);

	for (u16 i = 0; i < numParams; i++) {
		u32 paramOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);

		const u8* paramStart = offset + paramOffset;
		u8 typeVal = reader::readU8(paramStart);
		ParamType type;
		if (typeVal == 0)
			type = ParamType::U8;
		else if (typeVal == 1)
			type = ParamType::U16;
		else if (typeVal == 2)
			type = ParamType::S16;
		else if (typeVal == 5)
			type = ParamType::U32;
		else if (typeVal == 6)
			type = ParamType::F32;
		else if (typeVal == 8)
			type = ParamType::String;
		else if (typeVal == 9)
			type = ParamType::Null;
		else {
			return ResultInvalidParamType();
		}

		if (type == ParamType::Null) {
			u16 numStrings = reader::readU16(paramStart + 2, byteOrder);
			std::vector<u16> stringIndices;
			for (u16 j = 0; j < numStrings; j++)
				stringIndices.push_back(reader::readU16(paramStart + 4 + 2 * j, byteOrder));

			std::string name = reader::readString(paramStart + 4 + 2 * numStrings);
			params.push_back({ type, name, stringIndices });
		} else {
			std::string name = reader::readString(paramStart + 1);
			params.push_back({ type, name });
		}
	}

	return hk::ResultSuccess();
}

hk::Result Reader::TGL2::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u16 numStrings = reader::readU16(offset, byteOrder);
	for (u16 i = 0; i < numStrings; i++) {
		u32 stringOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);
		strings.push_back(reader::readString(offset + stringOffset));
	}

	return hk::ResultSuccess();
}

hk::Result Reader::SYL3::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numStyles = reader::readU32(offset, byteOrder);
	for (u32 i = 0; i < numStyles; i++) {
		const u8* styleStart = offset + 4 + 16 * i;
		u32 regionWidth = reader::readU32(styleStart + 0x0, byteOrder);
		u32 lineNum = reader::readU32(styleStart + 0x4, byteOrder);
		u32 fontIndex = reader::readU32(styleStart + 0x8, byteOrder);
		s32 baseColourIndex = reader::readS32(styleStart + 0xc, byteOrder);

		styles.push_back({ regionWidth, lineNum, fontIndex, baseColourIndex });
	}

	return hk::ResultSuccess();
}

hk::Result Reader::CTI1::read(const u8* offset, u32 size, util::ByteOrder byteOrder) {
	u32 numFilenames = reader::readU32(offset, byteOrder);
	for (u32 i = 0; i < numFilenames; i++) {
		u32 filenameOffset = reader::readU32(offset + 4 + 4 * i, byteOrder);
		filenames.push_back(reader::readString(offset + filenameOffset));
	}

	return hk::ResultSuccess();
}

} // namespace msbp
