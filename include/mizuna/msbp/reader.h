#pragma once

#include <map>
#include <vector>

#include "mizuna/msbp/common.h"
#include "mizuna/util.h"

namespace msbp {

class Reader {
public:
	using Colour = u32;

	class Attribute {};

	struct Style {
		u32 regionWidth;
		u32 lineNum;
		u32 fontIndex;
		s32 baseColourIndex;
	};

	struct TagParam {
		std::string name;
		ParamType type;
		std::vector<std::string> strings;
	};

	struct Tag {
		std::string name;
		std::vector<TagParam> params;
	};

	struct TagGroup {
		std::string name;
		std::vector<Tag> tags;
	};

	Reader(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	~Reader();

	util::ByteOrder getByteOrder() const { return mByteOrder; }

	u8 getVersion() const { return mVersion; }

	hk::Result read();

	const std::map<std::string, Colour>& getColours() const { return mColours; }

	const std::vector<Attribute>& getAttributes() const { return mAttributes; }

	const std::map<u16, TagGroup>& getTagGroups() const { return mTagGroups; }

	const std::map<std::string, Style>& getStyles() const { return mStyles; }

	const std::vector<std::string>& getFilenames() const { return mFilenames; }

private:
	struct Block {
		virtual ~Block() = default;
		virtual hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) = 0;
	};

	struct LabelBlock : public Block {
		struct Label {
			std::string name;
			u32 itemIndex;
		};

		~LabelBlock() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Label> labels;
	};

	struct CLR1 : public Block {
		using Colour = u32;

		~CLR1() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Colour> colours;
	};

	struct CLB1 : public LabelBlock {};

	struct ATI2 : public Block {
		struct Attribute {
			u8 type;
			u16 listIndex;
			u32 listOffset;
		};

		~ATI2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Attribute> attributes;
	};

	struct ALB1 : public LabelBlock {};

	struct ALI2 : public Block {
		using AttributeList = std::vector<std::string>;

		~ALI2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<AttributeList> lists;
	};

	struct TGG2 : public Block {
		struct Group {
			u16 id;
			std::string name;
			std::vector<u16> tagIndices;
		};

		~TGG2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Group> groups;
	};

	struct TAG2 : public Block {
		struct Tag {
			std::string name;
			std::vector<u16> paramIndices;
		};

		~TAG2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Tag> tags;
	};

	struct TGP2 : public Block {
		struct Param {
			Param(ParamType type, std::string name) : type(type), name(name) {}

			Param(ParamType type, std::string name, std::vector<u16> stringIndices) :
				type(type), name(name), stringIndices(stringIndices) {}

			ParamType type;
			std::string name;
			std::vector<u16> stringIndices;
		};

		~TGP2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Param> params;
	};

	struct TGL2 : public Block {
		~TGL2() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<std::string> strings;
	};

	struct SYL3 : public Block {
		struct Style {
			u32 regionWidth;
			u32 lineNum;
			u32 fontIndex;
			s32 baseColourIndex;
		};

		~SYL3() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<Style> styles;
	};

	struct SLB1 : public LabelBlock {};

	struct CTI1 : public Block {
		~CTI1() override = default;
		hk::Result read(const u8* offset, u32 size, util::ByteOrder byteOrder) override;

		std::vector<std::string> filenames;
	};

	hk::Result readHeader(const u8* offset);
	hk::Result readBlockHeader(const u8* offset, BlockType* type, u32* size);
	hk::Result readBlock(const u8* offset, BlockType type, u32 size);

	const std::vector<u8>& mContents;
	u8 mVersion;
	util::ByteOrder mByteOrder;
	Encoding mEncoding;
	u16 mNumBlocks;
	u32 mFileSize;
	std::map<BlockType, Block*> mBlocks;

	std::map<std::string, Colour> mColours;
	std::vector<Attribute> mAttributes;
	std::map<u16, TagGroup> mTagGroups;
	std::map<std::string, Style> mStyles;
	std::vector<std::string> mFilenames;
};

} // namespace msbp
