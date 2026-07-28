#include "mizuna/lms.h"

namespace lms {

size_t getCharSize(lms::Encoding encoding) {
	switch (encoding) {
	case Encoding::UTF8: return 1;
	case Encoding::UTF16: return 2;
	case Encoding::UTF32: return 4;
	}
}

} // namespace lms
