#pragma once

#include "afl/byml/common.h"
#include "afl/util.h"

namespace byml {

class Reader {
public:
	Reader();
	result_t init(const u8* fileData);
	result_t init(const Reader& other, const u32 offset);

	const std::string getHashString(u32 idx) const;
	const std::string getValueString(u32 idx) const;
	bool isExistHashString(const std::string& str) const;
	bool isExistStringValue(const std::string& str) const;

	NodeType getType() const;
	u32 getSize() const;
	bool hasKey(const std::string& key) const;

	result_t getTypeByIdx(NodeType* type, u32 idx) const;
	result_t getTypeByKey(NodeType* type, const std::string& key) const;
	const std::string getKeyByIdx(u32 idx) const;

	result_t getContainerByIdx(Reader* container, u32 idx) const;
	result_t getStringByIdx(std::string* out, u32 idx) const;
	result_t getBoolByIdx(bool* out, u32 idx) const;
	result_t getS32ByIdx(s32* out, u32 idx) const;
	result_t getF32ByIdx(f32* out, u32 idx) const;
	result_t getU32ByIdx(u32* out, u32 idx) const;
	result_t getS64ByIdx(s64* out, u32 idx) const;
	result_t getF64ByIdx(f64* out, u32 idx) const;
	result_t getU64ByIdx(u64* out, u32 idx) const;

	result_t getContainerByKey(Reader* container, const std::string& key) const;
	result_t getStringByKey(std::string* out, const std::string& key) const;
	result_t getBoolByKey(bool* out, const std::string& key) const;
	result_t getS32ByKey(s32* out, const std::string& key) const;
	result_t getF32ByKey(f32* out, const std::string& key) const;
	result_t getU32ByKey(u32* out, const std::string& key) const;
	result_t getS64ByKey(s64* out, const std::string& key) const;
	result_t getF64ByKey(f64* out, const std::string& key) const;
	result_t getU64ByKey(u64* out, const std::string& key) const;

	bool tryGetContainerByIdx(Reader* container, u32 idx) const {
		return getContainerByIdx(container, idx) ? false : true;
	}

	bool tryGetStringByIdx(std::string* out, u32 idx) const {
		return getStringByIdx(out, idx) ? false : true;
	}

	bool tryGetBoolByIdx(bool* out, u32 idx) const { return getBoolByIdx(out, idx) ? false : true; }

	bool tryGetS32ByIdx(s32* out, u32 idx) const { return getS32ByIdx(out, idx) ? false : true; }

	bool tryGetF32ByIdx(f32* out, u32 idx) const { return getF32ByIdx(out, idx) ? false : true; }

	bool tryGetU32ByIdx(u32* out, u32 idx) const { return getU32ByIdx(out, idx) ? false : true; }

	bool tryGetS64ByIdx(s64* out, u32 idx) const { return getS64ByIdx(out, idx) ? false : true; }

	bool tryGetF64ByIdx(f64* out, u32 idx) const { return getF64ByIdx(out, idx) ? false : true; }

	bool tryGetU64ByIdx(u64* out, u32 idx) const { return getU64ByIdx(out, idx) ? false : true; }

	bool tryGetContainerByKey(Reader* container, const std::string& key) const {
		return getContainerByKey(container, key) ? false : true;
	}

	bool tryGetStringByKey(std::string* out, const std::string& key) const {
		return getStringByKey(out, key) ? false : true;
	}

	bool tryGetBoolByKey(bool* out, const std::string& key) const {
		return getBoolByKey(out, key) ? false : true;
	}

	bool tryGetS32ByKey(s32* out, const std::string& key) const {
		return getS32ByKey(out, key) ? false : true;
	}

	bool tryGetF32ByKey(f32* out, const std::string& key) const {
		return getF32ByKey(out, key) ? false : true;
	}

	bool tryGetU32ByKey(u32* out, const std::string& key) const {
		return getU32ByKey(out, key) ? false : true;
	}

	bool tryGetS64ByKey(s64* out, const std::string& key) const {
		return getS64ByKey(out, key) ? false : true;
	}

	bool tryGetF64ByKey(f64* out, const std::string& key) const {
		return getF64ByKey(out, key) ? false : true;
	}

	bool tryGetU64ByKey(u64* out, const std::string& key) const {
		return getU64ByKey(out, key) ? false : true;
	}

private:
	struct Header {
		util::ByteOrder mByteOrder;
		u16 mVersion;
		u32 mHashKeyTableOffset = 0;
		u32 mStringValueTableOffset = 0;
		u32 mRootOffset = 0;

		u32 mHashKeyTableSize = 0;
		u32 mStringValueTableSize = 0;
	};

	result_t initHeader();
	void initKeyOrder();

	result_t getNodeByKey(const u8** offset, const std::string& key, NodeType expectedType) const;
	result_t getNodeByIdx(const u8** offset, u32 idx, NodeType expectedType) const;
	result_t getContainerOffsets(const u8** typeOffset, const u8** valueOffset, u32 idx) const;

	const u8* mFileData = nullptr;
	const u8* mOffset = nullptr;
	Header mHeader;

	std::vector<u32> mKeyOrder;
};

} // namespace byml
