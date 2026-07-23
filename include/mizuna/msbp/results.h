#pragma once

#include <hk/Result.h>

namespace msbp {

HK_RESULT_MODULE(4)
HK_DEFINE_RESULT_RANGE(Msbp, 0, 100)
HK_DEFINE_RESULT(InvalidTextEncoding, 0)
HK_DEFINE_RESULT(UnsupportedVersion, 1)
HK_DEFINE_RESULT(InvalidBlockType, 2)
HK_DEFINE_RESULT(BlockSizeOverrun, 3)
HK_DEFINE_RESULT(WrongHashBucketCount, 4)
HK_DEFINE_RESULT(DuplicateBlockType, 5)
HK_DEFINE_RESULT(InvalidParamType, 6)
HK_DEFINE_RESULT(BufferOverrun, 7)
HK_DEFINE_RESULT(Unimplemented, 8)

} // namespace msbp
