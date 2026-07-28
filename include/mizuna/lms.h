#pragma once

#include <codecvt>
#include <hk/types.h>
#include <locale>

namespace lms {

enum class Encoding : u8 {
	UTF8 = 0,
	UTF16 = 1,
	UTF32 = 2,
};

enum class ParamType : u8 {
	U8 = 0,
	U16 = 1,
	S16 = 2,
	U32 = 5,
	F32 = 6,
	String = 8,
	Null = 9,
};

struct String {
	// ~String() {
	// 	switch (encoding) {
	// 	case Encoding::UTF8: utf8.~basic_string(); break;
	// 	case Encoding::UTF16: utf16.~basic_string(); break;
	// 	case Encoding::UTF32: utf32.~basic_string(); break;
	// 	}
	// }

	String(const String& other) : encoding(other.encoding) {
		switch (encoding) {
		case Encoding::UTF8: utf8 = other.utf8; break;
		case Encoding::UTF16: utf16 = other.utf16; break;
		case Encoding::UTF32: utf32 = other.utf32; break;
		}
	}

	// String(String&& other) : encoding(other.encoding) {
	// 	switch (encoding) {
	// 	case Encoding::UTF8: utf8 = std::move(other.utf8); break;
	// 	case Encoding::UTF16: utf16 = std::move(other.utf16); break;
	// 	case Encoding::UTF32: utf32 = std::move(other.utf32); break;
	// 	}
	// 	other.isDead = true;
	// }

	String(lms::Encoding encoding) : encoding(encoding) {
		switch (encoding) {
		case Encoding::UTF8: utf8 = std::string(); break;
		case Encoding::UTF16: utf16 = std::u16string(); break;
		case Encoding::UTF32: utf32 = std::u32string(); break;
		}
	}

	String(const std::string str) : encoding(Encoding::UTF8), utf8(str) {}

	String(const std::u16string str) : encoding(Encoding::UTF16), utf16(str) {}

	String(const std::u32string str) : encoding(Encoding::UTF32), utf32(str) {}

	// String& operator=(String&& other) {
	// 	this->isDead = false;
	// 	encoding = other.encoding;
	// 	switch (encoding) {
	// 	case Encoding::UTF8: utf8 = std::move(other.utf8); break;
	// 	case Encoding::UTF16: utf16 = std::move(other.utf16); break;
	// 	case Encoding::UTF32: utf32 = std::move(other.utf32); break;
	// 	}
	// 	other.isDead = true;
	// 	return *this;
	// }

	// String& operator=(String other) { return *this; }

	size_t length() const {
		switch (encoding) {
		case Encoding::UTF8: return utf8.length();
		case Encoding::UTF16: return utf16.length();
		case Encoding::UTF32: return utf32.length();
		}
	}

	size_t sizeBytes() const {
		switch (encoding) {
		case Encoding::UTF8: return utf8.length() * sizeof(char);
		case Encoding::UTF16: return utf16.length() * sizeof(char16_t);
		case Encoding::UTF32: return utf32.length() * sizeof(char32_t);
		}
	}

	bool empty() const { return length() == 0; }

	void clear() {
		switch (encoding) {
		case Encoding::UTF8: utf8.clear(); break;
		case Encoding::UTF16: utf16.clear(); break;
		case Encoding::UTF32: utf32.clear(); break;
		}
	}

	void push_back(u32 char_) {
		switch (encoding) {
		case Encoding::UTF8: utf8.push_back((char)char_); break;
		case Encoding::UTF16: utf16.push_back((char16_t)char_); break;
		case Encoding::UTF32: utf32.push_back((char32_t)char_); break;
		}
	}

	std::string convertToUTF8() const {
		switch (encoding) {
		case lms::Encoding::UTF8: return utf8;
		case lms::Encoding::UTF16: {
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
			return conv.to_bytes(utf16);
			break;
		}
		case lms::Encoding::UTF32: {
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
			return conv.to_bytes(utf32);
			break;
		}
		}
	}

	Encoding encoding;

	std::string utf8;
	std::u16string utf16;
	std::u32string utf32;
};

size_t getCharSize(lms::Encoding encoding);

} // namespace lms
