#pragma once

// Yaz0 compression file format
// credit to http://amnoid.de/gc/yaz0.txt for helping me understand the format

#include <hk/types.h>
#include <vector>

namespace yaz0 {
enum class CompressionLevel : u8 {
	Auto = 0xff, // choose level automatically
	Lv0 = 0,     // no compression (~112.5% bigger than original file)
};

hk::Result decompress(
	std::vector<u8>& output, const std::vector<u8>& input, u32* outAlignment = nullptr
);
void compress(
	std::vector<u8>& output, const std::vector<u8>& input, u32 alignment,
	CompressionLevel level = CompressionLevel::Auto
);
} // namespace yaz0
