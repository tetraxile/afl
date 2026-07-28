#include "mizuna/msbt/reader.h"

#include <hk/Result.h>
#include <hk/ValueOrResult.h>
#include <hk/types.h>

#include "mizuna/msbt/results.h"
#include "mizuna/util.h"

namespace msbt {

Reader::~Reader() {
	for (auto [k, v] : mBlocks)
		delete v;
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

	if (mBlocks.contains(BlockType::LBL1) && mBlocks.contains(BlockType::TXT2)) {
		LabelBlock* lbl1 = static_cast<LabelBlock*>(mBlocks[BlockType::LBL1]);
		TXT2* txt2 = static_cast<TXT2*>(mBlocks[BlockType::TXT2]);

		for (const auto& label : lbl1->labels)
			mMessages[label.name] = &txt2->messages[label.itemIndex];
	}

	return hk::ResultSuccess();
}

hk::Result Reader::readHeader(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "MsgStdBn", 8));
	HK_TRY(reader::readByteOrder(&mByteOrder, offset + 8, 0xFEFF));

	u8 encodingVal = reader::readU8(offset + 0xc);
	if (encodingVal > 2) return ResultInvalidTextEncoding();
	mEncoding = static_cast<lms::Encoding>(encodingVal);

	mVersion = reader::readU8(offset + 0xd);
	if (mVersion != 3) return ResultUnsupportedVersion();

	mNumBlocks = reader::readU16(offset + 0xe, mByteOrder);
	mFileSize = reader::readU32(offset + 0x12, mByteOrder);

	return hk::ResultSuccess();
}

hk::Result Reader::readBlockHeader(const u8* offset, BlockType* type, u32* size) {
	const std::string signature = reader::readString(offset, 4);
	*size = reader::readU32(offset + 4, mByteOrder);

	if (signature == "LBL1")
		*type = BlockType::LBL1;
	else if (signature == "TXT2")
		*type = BlockType::TXT2;
	else if (signature == "ATR1")
		*type = BlockType::ATR1;
	else if (signature == "TSY1")
		*type = BlockType::TSY1;
	else
		return ResultInvalidBlockType();

	return hk::ResultSuccess();
}

hk::Result Reader::readBlock(const u8* offset, BlockType type, u32 size) {
	if (mBlocks.contains(type)) return ResultDuplicateBlockType();

	Block* block = nullptr;

	switch (type) {
	case BlockType::LBL1: block = new LabelBlock(offset, size, mEncoding); break;
	case BlockType::TXT2: block = new TXT2(offset, size, mEncoding, mMSBP); break;
	case BlockType::ATR1: block = new ATR1(offset, size, mEncoding); break;
	case BlockType::TSY1: block = new TSY1(offset, size, mEncoding); break;
	}

	if (!block) {
		fprintf(stderr, "fatal: unreachable\n");
		return hk::ResultFailed();
	}

	HK_TRY(block->read(mByteOrder));
	mBlocks[type] = block;

	return hk::ResultSuccess();
}

lms::String Reader::Block::readEncodedString(hk::Result* outResult, const u8* offset, size_t size) {
	size_t charSize = lms::getCharSize(encoding);
	if (util::roundUp(size, charSize) != size) {
		*outResult = ResultIncorrectStringLength();
		return lms::String(encoding);
	}
	size_t length = size / charSize;

	switch (encoding) {
	case lms::Encoding::UTF8: return lms::String(reader::readString(offset, length));
	case lms::Encoding::UTF16: return lms::String(reader::readU16String(offset, length));
	case lms::Encoding::UTF32: return lms::String(reader::readU32String(offset, length));
	}
}

hk::Result Reader::LabelBlock::read(util::ByteOrder byteOrder) {
	u32 numBuckets = reader::readU32(start, byteOrder);
	// if (numBuckets != 101) return ResultWrongHashBucketCount();

	const u8* labelsStart = start + 4 + 8 * numBuckets;
	u32 totalLabelCount = 0;
	for (u32 i = 0; i < numBuckets; i++) {
		totalLabelCount += reader::readU32(start + 4 + i * 8, byteOrder);
		u32 labelsOffset = reader::readU32(start + 4 + i * 8 + 4, byteOrder);
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

hk::Result Reader::TXT2::read(util::ByteOrder byteOrder) {
	u32 numMessages = reader::readU32(start, byteOrder);

	for (u32 i = 0; i < numMessages; i++) {
		u32 messageOffset = reader::readU32(start + 4 + 4 * i, byteOrder);
		Message string(encoding);
		HK_TRY(readString(&string, start + messageOffset, byteOrder));
		messages.push_back(string);
	}

	return hk::ResultSuccess();
}

hk::Result Reader::TXT2::readString(Message* out, const u8* offset, util::ByteOrder byteOrder) {
	lms::Encoding encoding = out->encoding;

	lms::String part(encoding);
	size_t index = 0;

	while (offset < start + size) {
		u32 char_;
		switch (encoding) {
		case lms::Encoding::UTF8:
			char_ = reader::readU8(offset);
			offset++;
			break;
		case lms::Encoding::UTF16:
			char_ = reader::readU16(offset, byteOrder);
			offset += 2;
			break;
		case lms::Encoding::UTF32:
			char_ = reader::readU32(offset, byteOrder);
			offset += 4;
			break;
		}

		// printf("  char: %#x\n", char_);

		if (char_ == 0x0e) {
			if (!part.empty()) {
				out->parts.insert({ index++, std::move(part) });
				part.clear();
			}

			u16 tagGroupID = reader::readU16(offset, byteOrder);
			u16 tagID = reader::readU16(offset + 2, byteOrder);

			if (!msbp.getTagGroups().contains(tagGroupID)) return ResultTagOutOfRange();
			auto tagGroup = msbp.getTagGroups().at(tagGroupID);

			if (tagID >= tagGroup.tags.size()) return ResultTagOutOfRange();
			auto tag = tagGroup.tags[tagID];

			u16 paramsSize = reader::readU16(offset + 4, byteOrder);
			offset += 6;

			std::vector<Message::Tag::Param*> params;
			for (const auto& param_ : tag.params) {
				using Param = Message::Tag::Param;

				Param* param;
				switch (param_.type) {
				case lms::ParamType::U8:
					param = new Param(param_.name, reader::readU8(offset));
					offset++;
					break;
				case lms::ParamType::U16:
					param = new Param(param_.name, reader::readU16(offset, byteOrder));
					offset += 2;
					break;
				case lms::ParamType::S16:
					param = new Param(param_.name, reader::readS16(offset, byteOrder));
					offset += 2;
					break;
				case lms::ParamType::U32:
					param = new Param(param_.name, reader::readU32(offset, byteOrder));
					offset += 4;
					break;
				case lms::ParamType::F32:
					param = new Param(param_.name, reader::readF32(offset, byteOrder));
					offset += 4;
					break;
				case lms::ParamType::String: {
					u16 size = reader::readU16(offset, byteOrder);
					hk::Result r;
					lms::String str = readEncodedString(&r, offset + 2, size);
					if (r.failed()) return r;

					param = new Param(param_.name, str);
					offset += 2 + size;
					break;
				}
				case lms::ParamType::Null: param = new Param(param->name); break;
				}

				params.push_back(std::move(param));
			}

			// out->tags.push_back(
			// 	{ std::move(tagGroup.name), std::move(tag.name), std::move(params) }
			// );
			Message::Tag tag_ = { tagGroup.name, tag.name, params };
			out->tags.insert({ index++, std::move(tag_) });
		} else if (char_ == 0x00 && !part.empty()) {
			out->parts.insert({ index++, std::move(part) });
			break;
		} else if (char_ == 0x0f) {
			HK_ABORT("encountered 0x0f character in MSBT string");
		} else if (char_ == 0x0a) {
			switch (part.encoding) {
			case lms::Encoding::UTF8: part.utf8.append("\\n"); break;
			case lms::Encoding::UTF16: part.utf16.append(u"\\n"); break;
			case lms::Encoding::UTF32: part.utf32.append(U"\\n"); break;
			}
		} else {
			switch (part.encoding) {
			case lms::Encoding::UTF8: part.utf8.push_back(static_cast<char>(char_)); break;
			case lms::Encoding::UTF16: part.utf16.push_back(static_cast<char16_t>(char_)); break;
			case lms::Encoding::UTF32: part.utf32.push_back(static_cast<char32_t>(char_)); break;
			}
		}
	}

	return hk::ResultSuccess();
}

hk::Result Reader::ATR1::read(util::ByteOrder byteOrder) {
	return ResultUnimplemented();
}

hk::Result Reader::TSY1::read(util::ByteOrder byteOrder) {
	return ResultUnimplemented();
}

std::string Message::toString() const {
	size_t index = 0;
	size_t size = tags.size() + parts.size();

	std::string out;

	for (size_t index = 0; index < size; index++) {
		if (tags.contains(index))
			out.append(tags.at(index).toString());
		else if (parts.contains(index))
			out.append(parts.at(index).convertToUTF8());
		else {
			HK_ABORT_UNLESS(!parts.contains(0), "missing part index %zu in MSBT message", index);
			lms::String a = parts.at(0);
			HK_ABORT("missing part index %zu in MSBT message %s", index, a.convertToUTF8().c_str());
		}
	}

	return out;
}

std::string Message::Tag::toString() const {
	std::string out = std::format("<{}.{}", groupName, tagName);
	for (Param* param : params)
		out += std::format(", {}", param->toString());
	out += ">";
	return out;
}

std::string Message::Tag::Param::toString() const {
	std::string out = std::format("{}: ", name);
	switch (type) {
	case lms::ParamType::U8: out += std::format("u8 {}", u8); break;
	case lms::ParamType::U16: out += std::format("u16 {}", u16); break;
	case lms::ParamType::S16: out += std::format("s16 {}", s16); break;
	case lms::ParamType::U32: out += std::format("u32 {}", u32); break;
	case lms::ParamType::F32: out += std::format("f32 {}", f32); break;
	case lms::ParamType::String: out += std::format("string \"{}\"", string.convertToUTF8()); break;
	case lms::ParamType::Null: out += std::format("null"); break;
	}
	return out;
}

} // namespace msbt
