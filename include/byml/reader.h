#ifndef BYML_READER_H
#define BYML_READER_H

#include "byml/common.h"
#include "util.h"

namespace byml {

class Reader {
public:
	struct Header {
		util::ByteOrder mByteOrder;
		u16 mVersion;
		u32 mHashKeyTableOffset = 0;
		u32 mStringValueTableOffset = 0;
	};

	Reader();
	result_t init(const u8* fileData);
	result_t init(const u8* fileData, const u8* offset);

	std::string get_hash_string(u32 idx) const;
	std::string get_value_string(u32 idx) const;

	NodeType get_type() const;
	u32 get_size() const;
	bool has_key(const std::string& key) const;

	result_t get_type_by_idx(NodeType* type, u32 idx) const;
	result_t get_type_by_key(NodeType* type, const std::string& key) const;
	result_t get_key_by_idx(u32* key, u32 idx) const;

	result_t get_container_by_idx(Reader* container, u32 idx) const;
	result_t get_string_by_idx(std::string* out, u32 idx) const;
	result_t get_bool_by_idx(bool* out, u32 idx) const;
	result_t get_s32_by_idx(s32* out, u32 idx) const;
	result_t get_f32_by_idx(f32* out, u32 idx) const;
	result_t get_u32_by_idx(u32* out, u32 idx) const;
	result_t get_s64_by_idx(s64* out, u32 idx) const;
	result_t get_f64_by_idx(f64* out, u32 idx) const;
	result_t get_u64_by_idx(u64* out, u32 idx) const;

	result_t get_container_by_key(Reader* container, const std::string& key) const;
	result_t get_string_by_key(std::string* out, const std::string& key) const;
	result_t get_bool_by_key(bool* out, const std::string& key) const;
	result_t get_s32_by_key(s32* out, const std::string& key) const;
	result_t get_f32_by_key(f32* out, const std::string& key) const;
	result_t get_u32_by_key(u32* out, const std::string& key) const;
	result_t get_s64_by_key(s64* out, const std::string& key) const;
	result_t get_f64_by_key(f64* out, const std::string& key) const;
	result_t get_u64_by_key(u64* out, const std::string& key) const;

private:
	result_t get_node_by_key(const u8** offset, const std::string& key, NodeType expectedType)
		const;
	result_t get_node_by_idx(const u8** offset, u32 idx, NodeType expectedType) const;

	result_t get_container_offsets(const u8** typeOffset, const u8** valueOffset, u32 idx) const;

	const u8* mFileData = nullptr;
	const u8* mOffset = nullptr;
	Header mHeader;
};

} // namespace byml

#endif
