#if defined(__GNUC__) // GCC, Clang or MinGW
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# pragma GCC diagnostic ignored "-Wdeprecated-literal-operator"
#endif

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

#if defined(__GNUC__) // GCC, Clang or MinGW
# pragma GCC diagnostic pop
#endif
