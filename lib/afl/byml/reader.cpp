#include "afl/byml/reader.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <numeric>

namespace byml {

Reader::Reader() {}

result_t Reader::initHeader() {
	result_t r;

	r = reader::readByteOrder(&mHeader.mByteOrder, mFileData, 0x4259);
	if (r) return r;

	mHeader.mVersion = reader::readU16(mFileData + 2, mHeader.mByteOrder);
	mHeader.mHashKeyTableOffset = reader::readU32(mFileData + 4, mHeader.mByteOrder);
	mHeader.mStringValueTableOffset = reader::readU32(mFileData + 8, mHeader.mByteOrder);
	assert(mHeader.mVersion == 2 || mHeader.mVersion == 3);

	mHeader.mHashKeyTableSize =
		reader::readU24(mFileData + mHeader.mHashKeyTableOffset + 1, mHeader.mByteOrder);
	mHeader.mStringValueTableSize =
		reader::readU24(mFileData + mHeader.mStringValueTableOffset + 1, mHeader.mByteOrder);

	return 0;
}

result_t Reader::init(const u8* fileData) {
	result_t r;

	mFileData = fileData;

	r = initHeader();
	if (r) return r;

	u32 rootOffset = reader::readU32(mFileData + 0xc, mHeader.mByteOrder);

	mOffset = mFileData + rootOffset;

	initKeyOrder();

	return 0;
}

result_t Reader::init(const u8* fileData, const u8* offset) {
	result_t r;

	mFileData = fileData;
	mOffset = offset;

	r = initHeader();
	if (r) return r;

	initKeyOrder();

	return 0;
}

void Reader::initKeyOrder() {
	if (getType() != NodeType::Hash) return;

	std::vector<u32> offsets;
	offsets.reserve(getSize());
	for (u32 i = 0; i < getSize(); i++) {
		NodeType type = (NodeType)reader::readU8(mOffset + 4 + i * 8 + 3);
		if (type == NodeType::Array || type == NodeType::Hash) {
			u32 offset = reader::readU32(mOffset + 8 + i * 8, mHeader.mByteOrder);
			offsets.push_back(offset);
		} else {
			offsets.push_back(0xffffffff);
		}
	}

	mKeyOrder.resize(getSize());
	std::iota(mKeyOrder.begin(), mKeyOrder.end(), 0);

	std::stable_sort(mKeyOrder.begin(), mKeyOrder.end(), [&offsets](size_t i1, size_t i2) {
		return offsets.at(i1) < offsets.at(i2);
	});
}

NodeType Reader::getType() const {
	return (NodeType)reader::readU8(mOffset);
}

u32 Reader::getSize() const {
	return reader::readU24(mOffset + 1, mHeader.mByteOrder);
}

std::string Reader::getHashString(u32 idx) const {
	if (idx >= mHeader.mHashKeyTableSize) return "(null)";

	const u8* base = mFileData + mHeader.mHashKeyTableOffset;
	const u8* strOffset = base + reader::readU32(base + 4 + (idx * 4), mHeader.mByteOrder);
	return reader::readString(strOffset);
}

std::string Reader::getValueString(u32 idx) const {
	if (idx >= mHeader.mStringValueTableSize) return "(null)";

	const u8* base = mFileData + mHeader.mStringValueTableOffset;
	const u8* strOffset = base + reader::readU32(base + 4 + (idx * 4), mHeader.mByteOrder);
	return reader::readString(strOffset);
}

bool Reader::hasKey(const std::string& key) const {
	if (getType() != NodeType::Hash) return false;

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) return true;
	}

	return false;
}

bool Reader::isExistHashString(const std::string& str) const {
	// TODO: implement binary search
	for (u32 i = 0; i < mHeader.mHashKeyTableSize; i++)
		if (util::isEqual(getHashString(i), str)) return true;

	return false;
}

bool Reader::isExistStringValue(const std::string& str) const {
	// TODO: implement binary search
	for (u32 i = 0; i < mHeader.mStringValueTableSize; i++)
		if (util::isEqual(getValueString(i), str)) return true;

	return false;
}

result_t Reader::getContainerOffsets(const u8** typeOffset, const u8** valueOffset, u32 idx) const {
	if (getType() == NodeType::Array) {
		*typeOffset = mOffset + 4 + idx;
		*valueOffset = mOffset + util::roundUp(4 + getSize(), 4) + 4 * idx;
	} else if (getType() == NodeType::Hash) {
		u32 hashIdx = mKeyOrder.at(idx);
		*typeOffset = mOffset + 7 + 8 * hashIdx;
		*valueOffset = mOffset + 8 + 8 * hashIdx;
	} else {
		return Error::WrongNodeType;
	}
	if (idx >= getSize()) return Error::OutOfBounds;

	return 0;
}

result_t Reader::getTypeByIdx(NodeType* type, u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = getContainerOffsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	*type = (NodeType)reader::readU8(typeOffset);
	return 0;
}

result_t Reader::getKeyByIdx(u32* key, u32 idx) const {
	if (getType() != NodeType::Hash) return Error::WrongNodeType;

	if (idx >= getSize()) return Error::OutOfBounds;

	u32 keyIdx = mKeyOrder.at(idx);
	*key = reader::readU24(mOffset + 4 + keyIdx * 8, mHeader.mByteOrder);

	return 0;
}

result_t Reader::getContainerByIdx(Reader* container, u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = getContainerOffsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	NodeType childType = (NodeType)reader::readU8(typeOffset);
	if (childType != NodeType::Array && childType != NodeType::Hash) return Error::WrongNodeType;

	u32 value = reader::readU32(valueOffset, mHeader.mByteOrder);
	container->init(mFileData, (const u8*)(mFileData + value));
	return 0;
}

result_t Reader::getNodeByIdx(const u8** offset, u32 idx, NodeType expectedType) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = getContainerOffsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	NodeType childType = (NodeType)reader::readU8(typeOffset);
	if (childType != expectedType) return Error::WrongNodeType;

	*offset = valueOffset;
	return 0;
}

result_t Reader::getStringByIdx(std::string* out, u32 idx) const {
	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::String);
	if (r) return r;

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = getValueString(value);
	return 0;
}

result_t Reader::getBoolByIdx(bool* out, u32 idx) const {
	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::Bool);
	if (r) return r;

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}

result_t Reader::getS32ByIdx(s32* out, u32 idx) const {
	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::S32);
	if (r) return r;

	*out = reader::readS32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getF32ByIdx(f32* out, u32 idx) const {
	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::F32);
	if (r) return r;

	*out = reader::readF32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getU32ByIdx(u32* out, u32 idx) const {
	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::U32);
	if (r) return r;

	*out = reader::readU32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getS64ByIdx(s64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::S64);
	if (r) return r;

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readS64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getF64ByIdx(f64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::F64);
	if (r) return r;

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readF64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getU64ByIdx(u64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByIdx(&offset, idx, NodeType::U64);
	if (r) return r;

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readU64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getTypeByKey(NodeType* type, const std::string& key) const {
	if (getType() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			*type = (NodeType)reader::readU8(childOffset + 3);
			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::getContainerByKey(Reader* container, const std::string& key) const {
	if (getType() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType type = (NodeType)reader::readU8(childOffset + 3);
			if (type != NodeType::Array && type != NodeType::Hash) return Error::WrongNodeType;

			u32 value = reader::readU32(childOffset + 4, mHeader.mByteOrder);
			container->init(mFileData, (const u8*)(mFileData + value));

			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::getNodeByKey(const u8** offset, const std::string& key, NodeType expectedType)
	const {
	if (getType() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType childType = (NodeType)reader::readU8(childOffset + 3);
			if (childType != expectedType) return Error::WrongNodeType;

			*offset = childOffset;
			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::getStringByKey(std::string* out, const std::string& key) const {
	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::String);
	if (r) return r;

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = getValueString(value);
	return 0;
}

result_t Reader::getBoolByKey(bool* out, const std::string& key) const {
	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::Bool);
	if (r) return r;

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}

result_t Reader::getS32ByKey(s32* out, const std::string& key) const {
	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::S32);
	if (r) return r;

	*out = reader::readS32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getF32ByKey(f32* out, const std::string& key) const {
	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::F32);
	if (r) return r;

	*out = reader::readF32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getU32ByKey(u32* out, const std::string& key) const {
	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::U32);
	if (r) return r;

	*out = reader::readU32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getS64ByKey(s64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::S64);
	if (r) return r;

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readS64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getF64ByKey(f64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::F64);
	if (r) return r;

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readF64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::getU64ByKey(u64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return Error::InvalidVersion;

	const u8* offset;
	result_t r = getNodeByKey(&offset, key, NodeType::U64);
	if (r) return r;

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readU64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

} // namespace byml
