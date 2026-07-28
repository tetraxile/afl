#pragma once

#include <map>
#include <vector>

#include "mizuna/lms.h"
#include "mizuna/msbp/reader.h"
#include "mizuna/msbt/common.h"
#include "mizuna/util.h"

namespace msbt {

struct Message {
	struct Tag {
		struct Param {
			Param(const std::string& name, u8 u8) : name(name), type(lms::ParamType::U8), u8(u8) {}

			Param(const std::string& name, u16 u16) :
				name(name), type(lms::ParamType::U16), u16(u16) {}

			Param(const std::string& name, s16 s16) :
				name(name), type(lms::ParamType::S16), s16(s16) {}

			Param(const std::string& name, u32 u32) :
				name(name), type(lms::ParamType::U32), u32(u32) {}

			Param(const std::string& name, f32 f32) :
				name(name), type(lms::ParamType::F32), f32(f32) {}

			Param(const std::string& name, lms::String string) :
				name(name), type(lms::ParamType::String), string(string) {}

			// Param(const std::string& name, lms::String&& string) :
			// 	name(name),
			// 	type(lms::ParamType::String),
			// 	string(std::forward<lms::String>(string)) {}

			Param(const std::string& name) : name(name), type(lms::ParamType::Null) {}

			~Param() {
				name.~basic_string();
				if (type == lms::ParamType::String) string.~String();
			}

			std::string toString() const;

			std::string name;
			lms::ParamType type;

			union {
				u8 u8;
				u16 u16;
				s16 s16;
				u32 u32;
				f32 f32;
				lms::String string;
			};
		};

		Tag(std::string groupName, std::string tagName, std::vector<Param*> params) :
			groupName(groupName), tagName(tagName), params(params) {}

		std::string toString() const;

		const std::string groupName;
		const std::string tagName;
		const std::vector<Param*> params;
	};

	Message(lms::Encoding encoding) : encoding(encoding) {}

	std::string toString() const;

	lms::Encoding encoding;

	std::map<size, lms::String> parts;
	std::map<size, Tag> tags;
};

class Reader {
public:
	Reader(const std::vector<u8>& fileContents, const msbp::Reader& msbp) :
		mContents(fileContents), mMSBP(msbp) {}

	~Reader();

	util::ByteOrder getByteOrder() const { return mByteOrder; }

	lms::Encoding getEncoding() const { return mEncoding; }

	const std::map<std::string, Message*>& getMessages() const { return mMessages; }

	u8 getVersion() const { return mVersion; }

	hk::Result read();

private:
	struct Block {
		Block(const u8* start, u32 size, lms::Encoding encoding) :
			start(start), size(size), encoding(encoding) {}

		virtual ~Block() = default;
		virtual hk::Result read(util::ByteOrder byteOrder) = 0;

		lms::String readEncodedString(hk::Result* outResult, const u8* offset, size_t size = -1);

	protected:
		const u8* start;
		u32 size;
		lms::Encoding encoding;
	};

	struct LabelBlock : public Block {
		struct Label {
			std::string name;
			u32 itemIndex;
		};

		LabelBlock(const u8* start, u32 size, lms::Encoding encoding) :
			Block(start, size, encoding) {}

		~LabelBlock() override = default;
		hk::Result read(util::ByteOrder byteOrder) override;

		std::vector<Label> labels;
	};

	struct TXT2 : public Block {
		TXT2(const u8* start, u32 size, lms::Encoding encoding, const msbp::Reader& msbp) :
			Block(start, size, encoding), msbp(msbp) {}

		~TXT2() override = default;

		hk::Result read(util::ByteOrder byteOrder) override;

		std::vector<Message> messages;

	private:
		hk::Result readString(Message* out, const u8* offset, util::ByteOrder byteOrder);

		const msbp::Reader& msbp;
	};

	struct ATR1 : public Block {
		ATR1(const u8* start, u32 size, lms::Encoding encoding) : Block(start, size, encoding) {}

		~ATR1() override = default;
		hk::Result read(util::ByteOrder byteOrder) override;
	};

	struct TSY1 : public Block {
		TSY1(const u8* start, u32 size, lms::Encoding encoding) : Block(start, size, encoding) {}

		~TSY1() override = default;
		hk::Result read(util::ByteOrder byteOrder) override;
	};

	hk::Result readHeader(const u8* offset);
	hk::Result readBlockHeader(const u8* offset, BlockType* type, u32* size);
	hk::Result readBlock(const u8* offset, BlockType type, u32 size);

	const std::vector<u8>& mContents;
	const msbp::Reader& mMSBP;
	u8 mVersion;
	util::ByteOrder mByteOrder;
	lms::Encoding mEncoding;
	u16 mNumBlocks;
	u32 mFileSize;
	std::map<BlockType, Block*> mBlocks;

	std::map<std::string, Message*> mMessages;
};

} // namespace msbt
