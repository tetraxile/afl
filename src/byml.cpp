#include "byml.h"

#include <cstdio>

BYML::BYML() {}

result_t BYML::init(const u8* fileData) {
	result_t r;

	mFileData = fileData;

	util::ByteOrder byteOrder;
	r = reader::read_byte_order(&mHeader.mByteOrder, mFileData, 0x4259);
	if (r) return r;

	mHeader.mVersion = reader::read_u16(mFileData + 2, mHeader.mByteOrder);
	mHeader.mHashKeyTableOffset = reader::read_u32(mFileData + 4, mHeader.mByteOrder);
	mHeader.mStringValueTableOffset = reader::read_u32(mFileData + 8, mHeader.mByteOrder);
	u32 rootOffset = reader::read_u32(mFileData + 0xc, mHeader.mByteOrder);

	mOffset = mFileData + rootOffset;

	return 0;
}

result_t BYML::init(const u8* fileData, const u8* offset) {
	result_t r;

	mFileData = fileData;
	mOffset = offset;

	util::ByteOrder byteOrder;
	r = reader::read_byte_order(&mHeader.mByteOrder, mFileData, 0x4259);
	if (r) return r;

	mHeader.mVersion = reader::read_u16(mFileData + 2, mHeader.mByteOrder);
	mHeader.mHashKeyTableOffset = reader::read_u32(mFileData + 4, mHeader.mByteOrder);
	mHeader.mStringValueTableOffset = reader::read_u32(mFileData + 8, mHeader.mByteOrder);

	return 0;
}

BYML::NodeType BYML::get_type() const {
	return (NodeType)reader::read_u8(mOffset, mHeader.mByteOrder);
}

u32 BYML::get_size() const {
	return reader::read_u24(mOffset + 1, mHeader.mByteOrder);
}

std::string BYML::get_hash_string(u32 idx) const {
	const u8* base = mFileData + mHeader.mHashKeyTableOffset;

	u32 size = reader::read_u24(base + 1, mHeader.mByteOrder);
	if (idx >= size)
		return "(null)";

	u32 addr = reader::read_u32(base + 4 + (idx * 4), mHeader.mByteOrder);

	const u8* strOffset = base + addr;
	return reader::read_string(strOffset);
}

std::string BYML::get_value_string(u32 idx) const {
	const u8* base = mFileData + mHeader.mStringValueTableOffset;

	u32 size = reader::read_u24(base + 1, mHeader.mByteOrder);
	if (idx >= size)
		return "(null)";

	u32 addr = reader::read_u32(base + 4 + (idx * 4), mHeader.mByteOrder);

	const u8* strOffset = base + addr;
	return reader::read_string(strOffset);
}

bool BYML::has_key(const std::string& key) const {
	if (get_type() != NodeType::Hash)
		return false;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i*8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx)))
			return true;
	}

	return false;
}

result_t BYML::get_container_by_idx(BYML* container, u32 idx) const {
	// TODO: allow indexing hash nodes by index?
	if (get_type() != NodeType::Array)
		return Error::WrongNodeType;

	if (idx >= get_size())
		return Error::OutOfBounds;

	// round up to multiple of 4
	u32 valuesOffset = (4 + get_size() + 3) & ~3;

	NodeType childType = (NodeType)reader::read_u8(mOffset + 4 + idx, mHeader.mByteOrder);
	if (childType != NodeType::Array && childType != NodeType::Hash)
		return Error::WrongNodeType;

	u32 value = reader::read_u32(mOffset + valuesOffset + 4*idx, mHeader.mByteOrder);
	container->init(mFileData, (const u8*)(mFileData + value));
	return 0;
}

result_t BYML::get_node_by_idx(const u8** offset, u32 idx, NodeType expectedType) const {
	// TODO: allow indexing hash nodes by index?
	if (get_type() != NodeType::Array)
		return Error::WrongNodeType;

	if (idx >= get_size())
		return Error::OutOfBounds;

	// round up to multiple of 4
	u32 valuesOffset = (4 + get_size() + 3) & ~3;

	NodeType childType = (NodeType)reader::read_u8(mOffset + 4 + idx, mHeader.mByteOrder);
	if (childType != expectedType)
		return Error::WrongNodeType;

	*offset = mOffset + valuesOffset + 4*idx;
	return 0;
}

result_t BYML::get_string_by_idx(std::string* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::String);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = get_value_string(value);
	return 0;
}

result_t BYML::get_bool_by_idx(bool* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::Bool);
	if (r) return r;

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}

result_t BYML::get_s32_by_idx(s32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::S32);
	if (r) return r;

	*out = reader::read_s32(offset, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_f32_by_idx(f32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::F32);
	if (r) return r;

	*out = reader::read_f32(offset, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_u32_by_idx(u32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::U32);
	if (r) return r;

	*out = reader::read_u32(offset, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_s64_by_idx(s64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::S64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_s64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_f64_by_idx(f64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::F64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_f64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_u64_by_idx(u64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::U64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_u64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_container_by_key(BYML* container, const std::string& key) const {
	const u8* offset;

	if (get_type() != NodeType::Hash)
		return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i*8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) {
			NodeType type = (NodeType)reader::read_u8(childOffset + 3, mHeader.mByteOrder);
			if (type != NodeType::Array && type != NodeType::Hash)
				return Error::WrongNodeType;

			u32 value = reader::read_u32(childOffset + 4, mHeader.mByteOrder);
			container->init(mFileData, (const u8*)(mFileData + value));

			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t BYML::get_node_by_key(const u8** offset, const std::string& key, NodeType expectedType) const {
	if (get_type() != NodeType::Hash)
		return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i*8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) {
			NodeType childType = (NodeType)reader::read_u8(childOffset + 3, mHeader.mByteOrder);
			if (childType != expectedType)
				return Error::WrongNodeType;

			*offset = childOffset;
			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t BYML::get_string_by_key(std::string* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::String);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = get_value_string(value);
	return 0;
}

result_t BYML::get_bool_by_key(bool* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::Bool);
	if (r) return r;
	
	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}


result_t BYML::get_s32_by_key(s32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::S32);
	if (r) return r;

	*out = reader::read_s32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_f32_by_key(f32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::F32);
	if (r) return r;

	*out = reader::read_f32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_u32_by_key(u32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::U32);
	if (r) return r;

	*out = reader::read_u32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_s64_by_key(s64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::S64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_s64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_f64_by_key(f64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::F64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_f64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t BYML::get_u64_by_key(u64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::U64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_u64(mFileData + value, mHeader.mByteOrder);
	return 0;
}
