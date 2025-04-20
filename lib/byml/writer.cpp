#include "byml/writer.h"

#include <cassert>

namespace byml {

void Writer::save(const std::string& filename, util::ByteOrder byteOrder) {
    if (byteOrder != util::ByteOrder::Little) {
        fprintf(stderr, "unimplemented big-endian byml writer\n");
        return;
    }

	std::vector<u8> outputBuffer;
	writer::write_u16_le(outputBuffer, 0, 0x4259);   // byte order mark
	writer::write_u16_le(outputBuffer, 2, mVersion); // version

	u32 hashKeyTableOffset = 0x10;
	u32 valueStringTableOffset = hashKeyTableOffset + mHashKeyStringTable.calc_size();
    u32 data64Offset = valueStringTableOffset + mValueStringTable.calc_size();
    u32 rootOffset = data64Offset + mData64.size() * 8;

    writer::write_u32_le(outputBuffer, 0x4, mHashKeyStringTable.is_empty() ? 0 : hashKeyTableOffset);
    writer::write_u32_le(outputBuffer, 0x8, mValueStringTable.is_empty() ? 0 : valueStringTableOffset);
    writer::write_u32_le(outputBuffer, 0xc, mContainerList.empty() ? 0 : rootOffset);

    mHashKeyStringTable.write(outputBuffer, hashKeyTableOffset);
    mValueStringTable.write(outputBuffer, valueStringTableOffset);
    
	u32 writePtr = data64Offset;
    for (auto* node : mData64) {
        node->set_offset(writePtr);
        node->write_data64(outputBuffer, writePtr);
        writePtr += 8;
    }

    writePtr = rootOffset;
    for (Container* container : mContainerList) {
        container->set_offset(writePtr);
        writePtr += container->calc_size();
    }

    for (Container* container : mContainerList) {
        container->write_container(outputBuffer);
    }

	util::write_file(filename, outputBuffer);
}

result_t Writer::push_array() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

    Array* child = new Array;

	// if stack is not empty, add array to container on top of stack
    if (mStackIdx > -1) {
        Container* top = mContainerStack[mStackIdx];
        if (top->mType != NodeType::Array) return Error::WrongNodeType;
        Array* parent = static_cast<Array*>(top);
        parent->mNodes.push_back(child);
    }

	// push array to stack and add to container list
    mContainerStack[++mStackIdx] = child;
    mContainerList.push_back(child);

    return 0;
}

result_t Writer::push_hash() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

    Hash* child = new Hash(mHashKeyStringTable);

	// if stack is not empty, add hash to container on top of stack
    if (mStackIdx > -1) {
        Container* top = mContainerStack[mStackIdx];
        if (top->mType != NodeType::Array) return Error::WrongNodeType;
        Array* parent = static_cast<Array*>(top);
        parent->mNodes.push_back(child);
    }

	// push hash to stack and add to container list
    mContainerStack[++mStackIdx] = child;
    mContainerList.push_back(child);

    return 0;
}

result_t Writer::push_array(const std::string& key) {
	// check if stack level is invalid, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;
    if (mStackIdx == -1) return Error::EmptyStack;

    Array* child = new Array;
    mHashKeyStringTable.add_string(key);

	// if stack is not empty, add array to container on top of stack
	Container* top = mContainerStack[mStackIdx];
    if (top->mType != NodeType::Hash) return Error::WrongNodeType;
    Hash* parent = static_cast<Hash*>(top);
    parent->mNodes.push_back(std::make_pair(key, child));

	// push array to stack and add to container list
    mContainerStack[++mStackIdx] = child;
    mContainerList.push_back(child);

    return 0;
}

result_t Writer::push_hash(const std::string& key) {
	// check if stack level is invalid, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;
    if (mStackIdx == -1) return Error::EmptyStack;

    Hash* child = new Hash(mHashKeyStringTable);
    mHashKeyStringTable.add_string(key);

	// if stack is not empty, add hash to container on top of stack
	Container* top = mContainerStack[mStackIdx];
    if (top->mType != NodeType::Hash) return Error::WrongNodeType;
    Hash* parent = static_cast<Hash*>(top);
    parent->mNodes.push_back(std::make_pair(key, child));

	// push hash to stack and add to container list
    mContainerStack[++mStackIdx] = child;
    mContainerList.push_back(child);

    return 0;
}

result_t Writer::pop() {
	// check if stack level is -1, if it is then return error
	if (mStackIdx == -1) return Error::EmptyStack;

	// decrease stack level
	mStackIdx--;

	return 0;
}

result_t Writer::add_node(Node* node) {
	if (mStackIdx == -1) return Error::EmptyStack;

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Array) return Error::WrongNodeType;

	Array* topArray = static_cast<Array*>(top);
	topArray->mNodes.push_back(node);
	return 0;
}

result_t Writer::add_node(const std::string& key, Node* node) {
	if (mStackIdx == -1) return Error::EmptyStack;

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Hash) return Error::WrongNodeType;

	Hash* topHash = static_cast<Hash*>(top);
    mHashKeyStringTable.add_string(key);
	topHash->mNodes.push_back(std::make_pair(key, node));
	return 0;
}

result_t Writer::add_string(const std::string& value) {
    mValueStringTable.add_string(value);
    return add_node(new String(value, mValueStringTable));
}

result_t Writer::add_bool(bool value) {
    return add_node(new Bool(value));
}

result_t Writer::add_s32(s32 value) {
    return add_node(new S32(value));
}

result_t Writer::add_f32(f32 value) {
    return add_node(new F32(value));
}

result_t Writer::add_u32(u32 value) {
    return add_node(new U32(value));
}

result_t Writer::add_s64(s64 value) {
    S64* node = new S64(value);
    mData64.push_back(node);
    return add_node(node);
}

result_t Writer::add_u64(u64 value) {
    U64* node = new U64(value);
    mData64.push_back(node);
    return add_node(node);
}

result_t Writer::add_f64(f64 value) {
    F64* node = new F64(value);
    mData64.push_back(node);
    return add_node(node);
}

result_t Writer::add_null() {
    return add_node(new Null);
}

result_t Writer::add_string(const std::string& key, const std::string& value) {
    mValueStringTable.add_string(value);
    return add_node(key, new String(value, mValueStringTable));
}

result_t Writer::add_bool(const std::string& key, bool value) {
    return add_node(key, new Bool(value));
}

result_t Writer::add_s32(const std::string& key, s32 value) {
    return add_node(key, new S32(value));
}

result_t Writer::add_f32(const std::string& key, f32 value) {
    return add_node(key, new F32(value));
}

result_t Writer::add_u32(const std::string& key, u32 value) {
    return add_node(key, new U32(value));
}

result_t Writer::add_s64(const std::string& key, s64 value) {
    S64* node = new S64(value);
    mData64.push_back(node);
    return add_node(key, node);
}

result_t Writer::add_u64(const std::string& key, u64 value) {
    U64* node = new U64(value);
    mData64.push_back(node);
    return add_node(key, node);
}

result_t Writer::add_f64(const std::string& key, f64 value) {
    F64* node = new F64(value);
    mData64.push_back(node);
    return add_node(key, node);
}

result_t Writer::add_null(const std::string& key) {
    return add_node(key, new Null);
}

void Writer::StringTable::add_string(const std::string& string) {
    mStrings.insert(string);
}

void Writer::StringTable::write(std::vector<u8>& outputBuffer, u32 offset) const {
    if (is_empty()) return;

    writer::write_u8(outputBuffer, offset, (u8)NodeType::StringTable);
    writer::write_u24_le(outputBuffer, offset + 1, size());

    u32 addrOffset = offset + 4;
    u32 strOffset = addrOffset + 4 * size() + 4;
    for (const std::string& string : mStrings) {
        writer::write_u32_le(outputBuffer, addrOffset, strOffset - offset);
        writer::write_string(outputBuffer, strOffset, string);
        strOffset += string.size() + 1;
        addrOffset += 4;
    }

    writer::write_u32_le(outputBuffer, offset + 4 + 4 * size(), strOffset - offset);
}

u32 Writer::StringTable::find(const std::string& string) const {
    s32 i = 0;
    for (const std::string& checkString : mStrings) {
        if (util::is_equal(string, checkString)) return i;
        i++;
    }

    assert(false && "string wasn't added to string table");

    return 0xffffff;
}

void Writer::Array::write_container(std::vector<u8>& outputBuffer) const {
    writer::write_u8(outputBuffer, mOffset, (u8)mType);
    writer::write_u24_le(outputBuffer, mOffset + 1, size());

    u32 typeOffset = mOffset + 4;
    for (Node* node : mNodes)
        writer::write_u8(outputBuffer, typeOffset++, (u8)node->mType);

    u32 valueOffset = mOffset + 4 + util::round_up(size(), 4);
    for (Node* node : mNodes) {
        node->write(outputBuffer, valueOffset);
        valueOffset += 4;
    }
}



void Writer::Hash::write_container(std::vector<u8>& outputBuffer) const {
    writer::write_u8(outputBuffer, mOffset, (u8)mType);
    writer::write_u24_le(outputBuffer, mOffset + 1, size());

    std::vector<std::pair<u32, Node*>> idxNodes;
    idxNodes.reserve(mNodes.size());
    for (const auto& [key, node] : mNodes) {
        idxNodes.push_back(std::make_pair(mHashKeyStringTable.find(key), node));
    }

    std::sort(idxNodes.begin(), idxNodes.end(), [](const auto& i1, const auto& i2) {
        return i1.first < i2.first;
    });

    u32 nodeOffset = mOffset + 4;
    for (const auto& [keyIdx, node] : idxNodes) {
        writer::write_u24_le(outputBuffer, nodeOffset, keyIdx);
        writer::write_u8(outputBuffer, nodeOffset + 3, (u8)node->mType);
        node->write(outputBuffer, nodeOffset + 4);
        nodeOffset += 8;
    }
}

}
