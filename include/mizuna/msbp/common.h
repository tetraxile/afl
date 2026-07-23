#pragma once

#include <hk/types.h>

namespace msbp {

enum class Encoding : u8 {
	UTF8 = 0,
	UTF16 = 1,
	UTF32 = 2,
};

enum class BlockType {
	CLR1, // colours
	CLB1, // colour labels
	ATI2, // attributes
	ALB1, // attribute labels
	ALI2, // attribute string lists
	TGG2, // tag groups
	TAG2, // tags
	TGP2, // tag parameters
	TGL2, // tag string lists
	SYL3, // styles
	SLB1, // style labels
	CTI1, // project contents
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

} // namespace msbp
