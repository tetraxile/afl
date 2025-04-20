#pragma once

#include <string>
#include <vector>
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
        virtual u32 calc_size() const = 0;
        virtual void write(std::vector<u8>& outputBuffer, u32 offset) const = 0;

		NodeType mType;
	};

	struct StringTable {
		void add_string(const std::string& string);
		void write(std::vector<u8>& outputBuffer, u32 offset) const;
		u32 find(const std::string& string) const;

		u32 size() const { return mStrings.size(); }
        bool is_empty() const { return size() == 0; }
        u32 calc_size() const {
            u32 headerSize = 8 + 4 * size();
            u32 stringsSize = 0;
            for (const auto& str : mStrings)
                stringsSize += str.size() + 1;
            return headerSize + util::round_up(stringsSize, 4);
        }

		std::set<std::string> mStrings;
	};

    struct Container : Node {
        Container(NodeType type) : Node(type) {}

        virtual size_t size() const = 0;
        virtual void write_container(std::vector<u8>& outputBuffer) const = 0;

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u32_le(outputBuffer, offset, mOffset);
        }

        void set_offset(u32 offset) { mOffset = offset; }

        u32 mOffset;
    };

    struct Array : Container {
        Array() : Container(NodeType::Array) {}

        void write_container(std::vector<u8>& outputBuffer) const override;

        size_t size() const override { return mNodes.size(); }

        u32 calc_size() const override {
            u32 listTypesSize = util::round_up(mNodes.size(), 4);
            return 4 + listTypesSize + mNodes.size() * 4;
        }

        std::vector<Node*> mNodes;
    };

    struct Hash : Container {
        Hash(const StringTable& hashKeyStringTable) : Container(NodeType::Hash), mHashKeyStringTable(hashKeyStringTable) {}

        void write_container(std::vector<u8>& outputBuffer) const override;

        size_t size() const override { return mNodes.size(); }

        u32 calc_size() const override {
            return 4 + 8 * mNodes.size();
        }

        const StringTable& mHashKeyStringTable;
        std::vector<std::pair<std::string, Node*>> mNodes;
    };

    struct ValueNode : Node {
        ValueNode(NodeType type) : Node(type) {}

        u32 calc_size() const override {
            return 4;
        }
    };

    struct String : ValueNode {
		String(const std::string& value, const StringTable& valueStringTable) :
            ValueNode(NodeType::String), mValueStringTable(valueStringTable), mValue(value) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            u32 index = mValueStringTable.find(mValue);
            writer::write_u32_le(outputBuffer, offset, index);
        }

        const StringTable& mValueStringTable;
        const std::string mValue;
    };

    struct Bool : ValueNode {
		Bool(bool value) : ValueNode(NodeType::Bool), mValue(value) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u32_le(outputBuffer, offset, mValue);
        }

        bool mValue;
    };

    struct S32 : ValueNode {
		S32(s32 value) : ValueNode(NodeType::S32), mValue(value) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_s32_le(outputBuffer, offset, mValue);
        }

        s32 mValue;
    };

    struct F32 : ValueNode {
		F32(f32 value) : ValueNode(NodeType::F32), mValue(value) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_f32_le(outputBuffer, offset, mValue);
        }

        f32 mValue;
    };

    struct U32 : ValueNode {
		U32(u32 value) : ValueNode(NodeType::U32), mValue(value) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u32_le(outputBuffer, offset, mValue);
        }

        u32 mValue;
    };

    struct Null : ValueNode {
		Null() : ValueNode(NodeType::Null) {}

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u32_le(outputBuffer, offset, 0);
        }
    };

    struct Value64Node : Node {
        Value64Node(NodeType type) : Node(type) {}

        virtual void write_data64(std::vector<u8>& outputBuffer, u32 offset) const = 0;

        u32 calc_size() const override {
            return 4;
        }

        void write(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u32_le(outputBuffer, offset, mOffset);
        }

        void set_offset(u32 offset) { mOffset = offset; }

        u32 mOffset;
    };

    struct S64 : Value64Node {
        S64(s64 value) : Value64Node(NodeType::S64), mValue(value) {}

        void write_data64(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_s64_le(outputBuffer, offset, mValue);
        }

        s64 mValue;
    };

    struct U64 : Value64Node {
        U64(u64 value) : Value64Node(NodeType::U64), mValue(value) {}

        void write_data64(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_u64_le(outputBuffer, offset, mValue);
        }

        u64 mValue;
    };

    struct F64 : Value64Node {
        F64(f64 value) : Value64Node(NodeType::F64), mValue(value) {}

        void write_data64(std::vector<u8>& outputBuffer, u32 offset) const override {
            writer::write_f64_le(outputBuffer, offset, mValue);
        }

        f64 mValue;
    };


public:
    Writer(u32 version) : mVersion(version) {}

    void save(const std::string& filename, util::ByteOrder byteOrder);

    result_t push_array();
    result_t push_hash();
    result_t push_array(const std::string& key);
    result_t push_hash(const std::string& key);
    result_t pop();

    result_t add_string(const std::string& value);
    result_t add_bool(bool value);
    result_t add_s32(s32 value);
    result_t add_f32(f32 value);
    result_t add_u32(u32 value);
    result_t add_s64(s64 value);
    result_t add_u64(u64 value);
    result_t add_f64(f64 value);
    result_t add_null();

    result_t add_string(const std::string& key, const std::string& value);
    result_t add_bool(const std::string& key, bool value);
    result_t add_s32(const std::string& key, s32 value);
    result_t add_f32(const std::string& key, f32 value);
    result_t add_u32(const std::string& key, u32 value);
    result_t add_s64(const std::string& key, s64 value);
    result_t add_u64(const std::string& key, u64 value);
    result_t add_f64(const std::string& key, f64 value);
    result_t add_null(const std::string& key);

private:
    result_t add_node(Node* node);
    result_t add_node(const std::string& key, Node* node);

	constexpr static u32 STACK_SIZE = 16;

	const u32 mVersion;
	s32 mStackIdx = -1;
	Container* mRoot = nullptr;
	std::array<Container*, STACK_SIZE> mContainerStack;
    std::vector<Container*> mContainerList;
	StringTable mHashKeyStringTable;
	StringTable mValueStringTable;
	std::vector<Value64Node*> mData64;
};

}
