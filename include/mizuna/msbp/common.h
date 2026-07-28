#pragma once

#include <hk/types.h>

namespace msbp {

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

} // namespace msbp
