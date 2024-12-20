#include "byml/writer.h"

#include <cassert>

namespace byml {

Writer::Writer(u32 version) : mVersion(version) {}

void Writer::init() {}

void Writer::save(const std::string& filename, util::ByteOrder byteOrder) const {
	if (byteOrder != util::ByteOrder::Little) {
		fprintf(stderr, "unimplemented big-endian byml writer\n");
		return;
	}

	std::vector<u8> outputBuffer;
	writer::write_u16_le(outputBuffer, 0, 0x4259);   // byte order mark
	writer::write_u16_le(outputBuffer, 2, mVersion); // version

	u32 writePtr = 0x10;

	u32 hashKeyTableOffset = mHashKeyStringTable.write(outputBuffer, &writePtr);
	u32 valueStringTableOffset = mValueStringTable.write(outputBuffer, &writePtr);

	writer::write_u32_le(outputBuffer, 0x4, hashKeyTableOffset);
	writer::write_u32_le(outputBuffer, 0x8, valueStringTableOffset);

	if (!mData64.empty()) writePtr = (writePtr + 7) & ~7;

	u32 data64Offset = writePtr;
	for (u64 value : mData64) {
		writer::write_u64_le(outputBuffer, writePtr, value);
		writePtr += 8;
	}

	if (mRoot) {
		writePtr = (writePtr + 3) & ~3;
		writer::write_u32_le(outputBuffer, 0xc, writePtr);
		mRoot->write(outputBuffer, &writePtr, data64Offset, mHashKeyStringTable, mValueStringTable);
	} else {
		writer::write_u32_le(outputBuffer, 0xc, 0); // no root offset
	}

	util::write_file(filename, outputBuffer);
}

result_t Writer::push_array() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

	// create root node if it has not been created
	if (mStackIdx == -1 && !mRoot) {
		Array* root = new Array; // XXX: leak
		mRoot = root;
		mContainerStack[++mStackIdx] = root;
		return 0;
	}

	// check if top of stack is array node, if not return error
	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Array) return Error::WrongNodeType;
	Array* parent = static_cast<Array*>(top);

	// increment stack level, create array and push it to the stack
	Array* child = new Array; // XXX: leak
	parent->mNodes.push_back(child);
	mContainerStack[++mStackIdx] = child;

	return 0;
}

result_t Writer::push_hash() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

	// create root node if it has not been created
	if (mStackIdx == -1 && !mRoot) {
		Hash* root = new Hash; // XXX: leak
		mRoot = root;
		mContainerStack[++mStackIdx] = root;
		return 0;
	}

	// check if top of stack is array node, if not return error
	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Array) return Error::WrongNodeType;
	Array* parent = static_cast<Array*>(top);

	// increment stack level, create hash and push it to the stack
	Hash* child = new Hash; // XXX: leak
	parent->mNodes.push_back(child);
	mContainerStack[++mStackIdx] = child;

	return 0;
}

result_t Writer::push_array(const std::string& key) {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

	// check if top of stack is hash node, if not return error
	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Hash) return Error::WrongNodeType;
	Hash* parent = static_cast<Hash*>(top);

	// increment stack level, create array and push it to the stack
	Array* child = new Array; // XXX: leak
	mHashKeyStringTable.add_string(key);
	parent->mNodes.push_back(std::make_pair(key, child));
	mContainerStack[++mStackIdx] = child;

	return 0;
}

result_t Writer::push_hash(const std::string& key) {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return Error::FullStack;

	// check if top of stack is hash node, if not return error
	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Hash) return Error::WrongNodeType;
	Hash* parent = static_cast<Hash*>(top);

	// increment stack level, create hash and push it to the stack
	Hash* child = new Hash; // XXX: leak
	mHashKeyStringTable.add_string(key);
	parent->mNodes.push_back(std::make_pair(key, child));
	mContainerStack[++mStackIdx] = child;

	return 0;
}

result_t Writer::pop() {
	// check if stack level is -1, if it is then return error
	if (mStackIdx == -1) return Error::EmptyStack;

	// decrease stack level
	mStackIdx--;

	return 0;
}

template <typename NodeT, typename InnerT>
result_t Writer::write_node(InnerT value) {
	if (mStackIdx == -1) return Error::EmptyStack;

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Array) return Error::WrongNodeType;

	Array* topArray = static_cast<Array*>(top);
	NodeT* node = new NodeT(value);
	topArray->mNodes.push_back(static_cast<Node*>(node));
	return 0;
}

result_t Writer::write_string(const std::string& value) {
	mValueStringTable.add_string(value);
	return write_node<String>(value);
}

result_t Writer::write_bool(bool value) {
	return write_node<Bool>(value);
}

result_t Writer::write_s32(s32 value) {
	return write_node<S32>(value);
}

result_t Writer::write_f32(f32 value) {
	return write_node<F32>(value);
}

result_t Writer::write_u32(u32 value) {
	return write_node<U32>(value);
}

result_t Writer::write_s64(s64 value) {
	u32 index = mData64.size();
	mData64.push_back(std::bit_cast<u64>(value));
	return write_node<S64>(index);
}

result_t Writer::write_f64(f64 value) {
	u32 index = mData64.size();
	mData64.push_back(std::bit_cast<u64>(value));
	return write_node<F64>(index);
}

result_t Writer::write_u64(u64 value) {
	u32 index = mData64.size();
	mData64.push_back(std::bit_cast<u64>(value));
	return write_node<U64>(index);
}

result_t Writer::write_null() {
	return write_node<Null>(0);
}

template <typename NodeT, typename InnerT>
result_t Writer::write_node(const std::string& key, InnerT value) {
	if (mStackIdx == -1) return Error::EmptyStack;

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Hash) return Error::WrongNodeType;

	Hash* topHash = static_cast<Hash*>(top);
	NodeT* node = new NodeT(value);
	mHashKeyStringTable.add_string(key);
	topHash->mNodes.push_back(std::make_pair(key, static_cast<Node*>(node)));
	return 0;
}

result_t Writer::write_string(const std::string& key, const std::string& value) {
	mValueStringTable.add_string(value);
	return write_node<String>(key, value);
}

result_t Writer::write_bool(const std::string& key, bool value) {
	return write_node<Bool>(key, value);
}

result_t Writer::write_s32(const std::string& key, s32 value) {
	return write_node<S32>(key, value);
}

result_t Writer::write_f32(const std::string& key, f32 value) {
	return write_node<F32>(key, value);
}

result_t Writer::write_u32(const std::string& key, u32 value) {
	return write_node<U32>(key, value);
}

result_t Writer::write_s64(const std::string& key, s64 value) {
	return write_node<S64>(key, value);
}

result_t Writer::write_f64(const std::string& key, f64 value) {
	return write_node<F64>(key, value);
}

result_t Writer::write_u64(const std::string& key, u64 value) {
	return write_node<U64>(key, value);
}

result_t Writer::write_null(const std::string& key) {
	return write_node<Null>(key, 0);
}

u32 Writer::StringTable::write(std::vector<u8>& output, u32* offset) const {
	assert(offset && "string table offset is null");

	if (mStrings.empty()) return 0;

	u32 writePtr = (*offset + 3) & ~3;
	u32 tableOffset = writePtr;

	writer::write_u8(output, writePtr, (u8)NodeType::StringTable);
	writer::write_u24_le(output, writePtr + 1, size());

	u32 offsetsOffset = writePtr + 4;
	writePtr = offsetsOffset + 4 * size() + 4;

	s32 i = 0;
	for (const std::string& string : mStrings) {
		writer::write_u32_le(output, offsetsOffset + 4 * i, writePtr - tableOffset);
		writer::write_string(output, writePtr, string);
		writePtr += string.size() + 1;
		i++;
	}

	writer::write_u32_le(output, offsetsOffset + 4 * size(), writePtr - tableOffset);

	*offset = writePtr;
	return tableOffset;
}

void Writer::StringTable::add_string(const std::string& string) {
	mStrings.insert(string);
}

u32 Writer::StringTable::find(const std::string& string) const {
	s32 i = 0;
	for (const std::string& test : mStrings) {
		if (util::is_equal(string, test)) return i;
		i++;
	}

	assert(false && "string wasn't added to string table");

	return 0xffffff;
}

void Writer::Container::write_node(
	std::vector<u8>& output, std::vector<std::pair<u32, Container*>>& deferredNodes, Node* node,
	const u32 offset, const u32 data64Offset, const StringTable& valueStringTable
) const {
	if (node->is_container_node()) {
		Container* container = static_cast<Container*>(node);
		deferredNodes.push_back(std::make_pair(offset, container));
	} else if (node->mType == NodeType::String) {
		String* stringNode = static_cast<String*>(node);
		stringNode->write(output, offset, valueStringTable);
	} else if (node->is_value_node()) {
		ValueNode* valueNode = static_cast<ValueNode*>(node);
		valueNode->write(output, offset);
	} else if (node->is_value64_node()) {
		Value64Node* value64Node = static_cast<Value64Node*>(node);
		value64Node->write(output, offset, data64Offset);
	}
}

void Writer::Array::write(
	std::vector<u8>& output, u32* offset, u32 data64Offset, const StringTable& hashKeyTable,
	const StringTable& valueStringTable
) const {
	std::vector<std::pair<u32, Container*>> deferredNodes;

	writer::write_u8(output, *offset, (u8)mType);
	writer::write_u24_le(output, *offset + 1, size());
	u32 valuesOffset = 4 + util::round_up(size(), 4);
	for (s32 i = 0; i < size(); i++) {
		Node* node = mNodes[i];
		u32 typeOffset = *offset + 4 + i;
		u32 valueOffset = *offset + valuesOffset + 4 * i;
		writer::write_u8(output, typeOffset, (u8)node->mType);
		write_node(output, deferredNodes, node, valueOffset, data64Offset, valueStringTable);
	}

	u32 offsetEntriesEnd = valuesOffset + 4 * size();

	*offset = *offset + offsetEntriesEnd;
	for (auto& [containerPointer, container] : deferredNodes) {
		writer::write_u32_le(output, containerPointer, *offset);
		container->write(output, offset, data64Offset, hashKeyTable, valueStringTable);
	}
}

void Writer::Hash::write(
	std::vector<u8>& output, u32* offset, u32 data64Offset, const StringTable& hashKeyTable,
	const StringTable& valueStringTable
) const {
	std::vector<std::pair<u32, Container*>> deferredNodes;

	writer::write_u8(output, *offset, (u8)mType);
	writer::write_u24_le(output, *offset + 1, size());
	for (s32 i = 0; i < size(); i++) {
		u32 entryOffset = *offset + 4 + 8 * i;
		auto [key, node] = mNodes[i];
		u32 keyIdx = hashKeyTable.find(key);
		writer::write_u24_le(output, entryOffset, keyIdx);
		writer::write_u8(output, entryOffset + 3, (u8)node->mType);
		write_node(output, deferredNodes, node, entryOffset + 4, data64Offset, valueStringTable);
	}

	u32 offsetEntriesEnd = 4 + 8 * size();

	*offset = *offset + offsetEntriesEnd;
	for (auto& [containerPointer, container] : deferredNodes) {
		writer::write_u32_le(output, containerPointer, *offset);
		container->write(output, offset, data64Offset, hashKeyTable, valueStringTable);
	}
}

void Writer::String::write(std::vector<u8>& output, u32 offset, const StringTable& valueStringTable)
	const {
	u32 index = valueStringTable.find(mValue);
	writer::write_u32_le(output, offset, index);
}

void Writer::Bool::write(std::vector<u8>& output, u32 offset) const {
	writer::write_u32_le(output, offset, mValue);
}

void Writer::S32::write(std::vector<u8>& output, u32 offset) const {
	writer::write_s32_le(output, offset, mValue);
}

void Writer::F32::write(std::vector<u8>& output, u32 offset) const {
	writer::write_f32_le(output, offset, mValue);
}

void Writer::U32::write(std::vector<u8>& output, u32 offset) const {
	writer::write_u32_le(output, offset, mValue);
}

void Writer::S64::write(std::vector<u8>& output, u32 offset, u32 data64Offset) const {
	writer::write_u32_le(output, offset, data64Offset + mIndex * 8);
}

void Writer::F64::write(std::vector<u8>& output, u32 offset, u32 data64Offset) const {
	writer::write_u32_le(output, offset, data64Offset + mIndex * 8);
}

void Writer::U64::write(std::vector<u8>& output, u32 offset, u32 data64Offset) const {
	writer::write_u32_le(output, offset, data64Offset + mIndex * 8);
}

void Writer::Null::write(std::vector<u8>& output, u32 offset) const {
	writer::write_u32_le(output, offset, 0);
}

} // namespace byml
