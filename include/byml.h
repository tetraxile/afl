#ifndef BYML_H
#define BYML_H

#include "util.h"

class BYML {
public:
	enum class NodeType : u8 {
		String = 0xa0,
		Binary = 0xa1,
		Array = 0xc0,
		Hash = 0xc1,
		StringTable = 0xc2,
		Bool = 0xd0,
		S32 = 0xd1,
		F32 = 0xd2,
		U32 = 0xd3,
		S64 = 0xd4,
		U64 = 0xd5,
		F64 = 0xd6,
		Null = 0xff,
	};

	struct Header {
		util::ByteOrder mByteOrder;
		u16 mVersion;
		u32 mHashKeyTableOffset = 0;
		u32 mStringValueTableOffset = 0;
	};

	enum Error : result_t {
		WrongNodeType = 0x101,
		InvalidKey = 0x102,
		OutOfBounds = 0x103,
	};

	BYML();
	result_t init(const u8* fileData);
	result_t init(const u8* fileData, const u8* offset);

	std::string get_hash_string(u32 idx) const;
	std::string get_value_string(u32 idx) const;

	NodeType get_type() const;
	u32 get_size() const;
	bool has_key(const std::string& key) const;

	result_t get_type_by_idx(NodeType* type, u32 idx) const;
	result_t get_type_by_key(NodeType* type, const std::string& key) const;

	result_t get_container_by_idx(BYML* container, u32 idx) const;
	result_t get_string_by_idx(std::string* out, u32 idx) const;
	result_t get_bool_by_idx(bool* out, u32 idx) const;
	result_t get_s32_by_idx(s32* out, u32 idx) const;
	result_t get_f32_by_idx(f32* out, u32 idx) const;
	result_t get_u32_by_idx(u32* out, u32 idx) const;
	result_t get_s64_by_idx(s64* out, u32 idx) const;
	result_t get_f64_by_idx(f64* out, u32 idx) const;
	result_t get_u64_by_idx(u64* out, u32 idx) const;

	result_t get_container_by_key(BYML* container, const std::string& key) const;
	result_t get_string_by_key(std::string* out, const std::string& key) const;
	result_t get_bool_by_key(bool* out, const std::string& key) const;
	result_t get_s32_by_key(s32* out, const std::string& key) const;
	result_t get_f32_by_key(f32* out, const std::string& key) const;
	result_t get_u32_by_key(u32* out, const std::string& key) const;
	result_t get_s64_by_key(s64* out, const std::string& key) const;
	result_t get_f64_by_key(f64* out, const std::string& key) const;
	result_t get_u64_by_key(u64* out, const std::string& key) const;

private:
	result_t get_node_by_key(const u8** offset, const std::string& key, NodeType expectedType) const;
	result_t get_node_by_idx(const u8** offset, u32 idx, NodeType expectedType) const;

	const u8* mFileData = nullptr;
	const u8* mOffset = nullptr;
	Header mHeader;
};

#endif
