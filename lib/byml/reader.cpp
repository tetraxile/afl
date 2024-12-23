#include "byml/reader.h"

#include <cassert>
#include <cstdio>

namespace byml {

Reader::Reader() {}

result_t Reader::init(const u8* fileData) {
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

result_t Reader::init(const u8* fileData, const u8* offset) {
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

NodeType Reader::get_type() const {
	return (NodeType)reader::read_u8(mOffset);
}

u32 Reader::get_size() const {
	return reader::read_u24(mOffset + 1, mHeader.mByteOrder);
}

std::string Reader::get_hash_string(u32 idx) const {
	const u8* base = mFileData + mHeader.mHashKeyTableOffset;

	u32 size = reader::read_u24(base + 1, mHeader.mByteOrder);
	if (idx >= size) return "(null)";

	u32 addr = reader::read_u32(base + 4 + (idx * 4), mHeader.mByteOrder);

	const u8* strOffset = base + addr;
	return reader::read_string(strOffset);
}

std::string Reader::get_value_string(u32 idx) const {
	const u8* base = mFileData + mHeader.mStringValueTableOffset;

	u32 size = reader::read_u24(base + 1, mHeader.mByteOrder);
	if (idx >= size) return "(null)";

	u32 addr = reader::read_u32(base + 4 + (idx * 4), mHeader.mByteOrder);

	const u8* strOffset = base + addr;
	return reader::read_string(strOffset);
}

bool Reader::has_key(const std::string& key) const {
	if (get_type() != NodeType::Hash) return false;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) return true;
	}

	return false;
}

result_t Reader::get_container_offsets(const u8** typeOffset, const u8** valueOffset, u32 idx)
	const {
	if (get_type() == NodeType::Array) {
		*typeOffset = mOffset + 4 + idx;
		*valueOffset = mOffset + util::round_up(4 + get_size(), 4) + 4 * idx;
	} else if (get_type() == NodeType::Hash) {
		*typeOffset = mOffset + 7 + 8 * idx;
		*valueOffset = mOffset + 8 + 8 * idx;
	} else {
		return Error::WrongNodeType;
	}
	if (idx >= get_size()) return Error::OutOfBounds;

	return 0;
}

result_t Reader::get_type_by_idx(NodeType* type, u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = get_container_offsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	*type = (NodeType)reader::read_u8(typeOffset);
	return 0;
}

result_t Reader::get_key_by_idx(u32* key, u32 idx) const {
	if (get_type() != NodeType::Hash) return Error::WrongNodeType;

	if (idx >= get_size()) return Error::OutOfBounds;

	*key = reader::read_u24(mOffset + 4 + idx * 8, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_container_by_idx(Reader* container, u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = get_container_offsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	NodeType childType = (NodeType)reader::read_u8(typeOffset);
	if (childType != NodeType::Array && childType != NodeType::Hash) return Error::WrongNodeType;

	u32 value = reader::read_u32(valueOffset, mHeader.mByteOrder);
	container->init(mFileData, (const u8*)(mFileData + value));
	return 0;
}

result_t Reader::get_node_by_idx(const u8** offset, u32 idx, NodeType expectedType) const {
	const u8 *typeOffset, *valueOffset;
	result_t r = get_container_offsets(&typeOffset, &valueOffset, idx);
	if (r) return r;

	NodeType childType = (NodeType)reader::read_u8(typeOffset);
	if (childType != expectedType) return Error::WrongNodeType;

	*offset = valueOffset;
	return 0;
}

result_t Reader::get_string_by_idx(std::string* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::String);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = get_value_string(value);
	return 0;
}

result_t Reader::get_bool_by_idx(bool* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::Bool);
	if (r) return r;

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}

result_t Reader::get_s32_by_idx(s32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::S32);
	if (r) return r;

	*out = reader::read_s32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_f32_by_idx(f32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::F32);
	if (r) return r;

	*out = reader::read_f32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_u32_by_idx(u32* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::U32);
	if (r) return r;

	*out = reader::read_u32(offset, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_s64_by_idx(s64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::S64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_s64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_f64_by_idx(f64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::F64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_f64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_u64_by_idx(u64* out, u32 idx) const {
	const u8* offset;
	result_t r = get_node_by_idx(&offset, idx, NodeType::U64);
	if (r) return r;

	u32 value = reader::read_u32(offset, mHeader.mByteOrder);
	*out = reader::read_u64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_type_by_key(NodeType* type, const std::string& key) const {
	if (get_type() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) {
			*type = (NodeType)reader::read_u8(childOffset + 3);
			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::get_container_by_key(Reader* container, const std::string& key) const {
	if (get_type() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) {
			NodeType type = (NodeType)reader::read_u8(childOffset + 3);
			if (type != NodeType::Array && type != NodeType::Hash) return Error::WrongNodeType;

			u32 value = reader::read_u32(childOffset + 4, mHeader.mByteOrder);
			container->init(mFileData, (const u8*)(mFileData + value));

			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::get_node_by_key(const u8** offset, const std::string& key, NodeType expectedType)
	const {
	if (get_type() != NodeType::Hash) return Error::WrongNodeType;

	// TODO: implement binary search
	for (u32 i = 0; i < get_size(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::read_u24(childOffset, mHeader.mByteOrder);
		if (util::is_equal(key, get_hash_string(keyIdx))) {
			NodeType childType = (NodeType)reader::read_u8(childOffset + 3);
			if (childType != expectedType) return Error::WrongNodeType;

			*offset = childOffset;
			return 0;
		}
	}

	return Error::InvalidKey;
}

result_t Reader::get_string_by_key(std::string* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::String);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = get_value_string(value);
	return 0;
}

result_t Reader::get_bool_by_key(bool* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::Bool);
	if (r) return r;

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = value != 0;
	return 0;
}

result_t Reader::get_s32_by_key(s32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::S32);
	if (r) return r;

	*out = reader::read_s32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_f32_by_key(f32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::F32);
	if (r) return r;

	*out = reader::read_f32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_u32_by_key(u32* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::U32);
	if (r) return r;

	*out = reader::read_u32(offset + 4, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_s64_by_key(s64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::S64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_s64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_f64_by_key(f64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::F64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_f64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

result_t Reader::get_u64_by_key(u64* out, const std::string& key) const {
	const u8* offset;
	result_t r = get_node_by_key(&offset, key, NodeType::U64);
	if (r) return r;

	u32 value = reader::read_u32(offset + 4, mHeader.mByteOrder);
	*out = reader::read_u64(mFileData + value, mHeader.mByteOrder);
	return 0;
}

} // namespace byml
