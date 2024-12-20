#ifndef BYML_WRITER_H
#define BYML_WRITER_H

#include <array>
#include <set>

#include "byml/common.h"
#include "util.h"

namespace byml {

class Writer {
private:
	struct Node {
		Node(NodeType type) : mType(type) {}

		bool is_container_node() const {
			return mType == NodeType::Array || mType == NodeType::Hash;
		}

		bool is_value_node() const {
			return mType == NodeType::Bool || mType == NodeType::S32 || mType == NodeType::F32 ||
			       mType == NodeType::U32 || mType == NodeType::Null;
		}

		bool is_value64_node() const {
			return mType == NodeType::S64 || mType == NodeType::F64 || mType == NodeType::U64;
		}

		virtual ~Node() {}

		NodeType mType;
	};

	struct StringTable {
		void add_string(const std::string& string);
		u32 write(std::vector<u8>& output, u32* offset) const;
		u32 find(const std::string& string) const;

		u32 size() const { return mStrings.size(); }

		std::set<std::string> mStrings;
	};

	struct String : Node {
		String(const std::string& value) : Node(NodeType::String), mValue(value) {}

		void write(std::vector<u8>& output, u32 offset, const StringTable& valueStringTable) const;

		std::string mValue;
	};

	struct ValueNode : Node {
		ValueNode(NodeType nodeType) : Node(nodeType) {}

		virtual void write(std::vector<u8>& output, u32 offset) const = 0;
	};

	struct Bool : ValueNode {
		Bool(bool value) : ValueNode(NodeType::Bool), mValue(value) {}

		void write(std::vector<u8>& output, u32 offset) const override;

		bool mValue;
	};

	struct S32 : ValueNode {
		S32(s32 value) : ValueNode(NodeType::S32), mValue(value) {}

		void write(std::vector<u8>& output, u32 offset) const override;

		s32 mValue;
	};

	struct U32 : ValueNode {
		U32(u32 value) : ValueNode(NodeType::U32), mValue(value) {}

		void write(std::vector<u8>& output, u32 offset) const override;

		u32 mValue;
	};

	struct F32 : ValueNode {
		F32(f32 value) : ValueNode(NodeType::F32), mValue(value) {}

		void write(std::vector<u8>& output, u32 offset) const override;

		f32 mValue;
	};

	struct Null : ValueNode {
		Null(u32) : ValueNode(NodeType::Null) {}

		void write(std::vector<u8>& output, u32 offset) const override;
	};

	struct Value64Node : Node {
		Value64Node(NodeType nodeType) : Node(nodeType) {}

		virtual void write(std::vector<u8>& output, u32 offset, u32 data64Offset) const = 0;
	};

	struct S64 : Value64Node {
		S64(u32 index) : Value64Node(NodeType::S64), mIndex(index) {}

		void write(std::vector<u8>& output, u32 offset, u32 data64Offset) const override;

		u32 mIndex;
	};

	struct U64 : Value64Node {
		U64(u32 index) : Value64Node(NodeType::U64), mIndex(index) {}

		void write(std::vector<u8>& output, u32 offset, u32 data64Offset) const override;

		u32 mIndex;
	};

	struct F64 : Value64Node {
		F64(u32 index) : Value64Node(NodeType::F64), mIndex(index) {}

		void write(std::vector<u8>& output, u32 offset, u32 data64Offset) const override;

		u32 mIndex;
	};

	struct Container : Node {
		Container(NodeType type) : Node(type) {}

		void write_node(
			std::vector<u8>& output, std::vector<std::pair<u32, Container*>>& deferredNodes,
			Node* node, const u32 offset, const u32 data64Offset,
			const StringTable& valueStringTable
		) const;

		virtual void write(
			std::vector<u8>& output, u32* offset, u32 data64Offset, const StringTable& hashKeyTable,
			const StringTable& valueStringTable
		) const = 0;

		virtual u32 size() const = 0;
	};

	struct Array : Container {
		Array() : Container(NodeType::Array) {}

		~Array() {
			for (auto node : mNodes)
				delete node;
		}

		u32 size() const override { return mNodes.size(); }

		void write(
			std::vector<u8>& output, u32* offset, u32 data64Offset, const StringTable& hashKeyTable,
			const StringTable& valueStringTable
		) const override;

		std::vector<Node*> mNodes;
	};

	struct Hash : Container {
		Hash() : Container(NodeType::Hash) {}

		~Hash() {}

		void write(
			std::vector<u8>& output, u32* offset, u32 data64Offset, const StringTable& hashKeyTable,
			const StringTable& valueStringTable
		) const override;

		u32 size() const override { return mNodes.size(); }

		std::vector<std::pair<std::string, Node*>> mNodes;
	};

public:
	Writer(u32 version);
	// ~Writer();
	void init();
	void save(const std::string& filename, util::ByteOrder byteOrder) const;

	result_t push_array();
	result_t push_hash();
	result_t push_array(const std::string& key);
	result_t push_hash(const std::string& key);
	result_t pop();

	result_t write_string(const std::string& value);
	result_t write_bool(bool value);
	result_t write_s32(s32 value);
	result_t write_f32(f32 value);
	result_t write_u32(u32 value);
	result_t write_s64(s64 value);
	result_t write_f64(f64 value);
	result_t write_u64(u64 value);
	result_t write_null();

	result_t write_string(const std::string& key, const std::string& value);
	result_t write_bool(const std::string& key, bool value);
	result_t write_s32(const std::string& key, s32 value);
	result_t write_f32(const std::string& key, f32 value);
	result_t write_u32(const std::string& key, u32 value);
	result_t write_s64(const std::string& key, s64 value);
	result_t write_f64(const std::string& key, f64 value);
	result_t write_u64(const std::string& key, u64 value);
	result_t write_null(const std::string& key);

	// TODO: write_null

private:
	template <typename NodeT, typename InnerT>
	result_t write_node(InnerT value);

	template <typename NodeT, typename InnerT>
	result_t write_node(const std::string& key, InnerT value);

	constexpr static u32 STACK_SIZE = 16;

	u32 mVersion;
	s32 mStackIdx = -1;
	Container* mRoot = nullptr;
	std::array<Container*, STACK_SIZE> mContainerStack;
	StringTable mHashKeyStringTable;
	StringTable mValueStringTable;
	std::vector<u64> mData64;
};

} // namespace byml

#endif
