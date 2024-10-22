#ifndef YAZ0_H
#define YAZ0_H

// Yaz0 compression file format
// credit to http://amnoid.de/gc/yaz0.txt for helping me understand the format
//
// ===== HEADER =====
//
//  0 1 2 3 4 5 6 7 8 9 a b c d e f
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | magic |  size | align |  PAD  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// magic: 4 bytes
//   file signature 'Yaz0'
//
// size: 4 bytes
//   uncompressed size of the file. big endian.
//
// alignment: 4 bytes
//   
// padding: 4 bytes
//
// 
// ===== COMPRESSION =====
//
// first, read one "code" byte. this byte's 8 bits will determine what to do
// for the next 8 operations. read the bits from most significant to least
// significant. if a bit is 1, copy one byte directly from the input to the
// output buffer. if the bit is 0, compressed data is ahead. read 2 bytes as
// follows:
//
//      byte 1          byte 2
//  0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | count |         offset        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// count: 4 bits
//   number of bytes to read.
//   if count is 0, then read another byte and add 0x12 to it.
//   otherwise, just add 2.
// 
// offset: 12 bits
//   offset from write pointer in output buffer to read from.
//   add 1 to the value, and then subtract it from the write pointer.
// 
//
// subtract `offset` from the write pointer in the output buffer, and then
// copy `count` bytes from there to the end of the output buffer.
//
// note: `count` may be larger than `offset`, meaning the copy source and
// destination overlap.
//
// note: not all 8 bits of the "code" byte need to be used. if the write
// pointer exceeds the specified uncompressed file size, any remaining bits
// of the current code byte will be skipped.
//
// info: `count` only starts at 3 because a compressed chunk of `count` 2 or
// less would take more bytes than just copying due to the 2 bytes of info
// for compressed data.

#include <vector>

#include "types.h"

namespace yaz0 {
s32 decompress(const std::vector<u8>& input, std::vector<u8>& output);
void compress(const std::vector<u8>& input, std::vector<u8>& output, u32 alignment);
}

#endif
