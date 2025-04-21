#pragma once

#include "types.h"
#include "byml/common.h"

constexpr const char* result_to_string(result_t r) {
	switch (r) {
	case util::Error::BadSignature: return "bad signature";
	case util::Error::BadByteOrder: return "invalid byte order";
	case util::Error::FileError: return "file error";
	case util::Error::FileNotFound: return "file not found";
	case util::Error::DirNotFound: return "directory not found";
    case byml::Error::WrongNodeType: return "byml: wrong node type";
    case byml::Error::InvalidKey: return "byml: invalid key";
    case byml::Error::OutOfBounds: return "byml: out of bounds";
    case byml::Error::EmptyStack: return "byml: empty stack";
    case byml::Error::FullStack: return "byml: full stack";
    case byml::Error::InvalidVersion: return "byml: invalid version";
	}
	return "(unknown)";
}
