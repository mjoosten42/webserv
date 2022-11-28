#include <stdint.h> // uint8_t

#include "ConfigParser.hpp"
#include "Listener.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	const std::string	str(reinterpret_cast<const char*>(Data), Size);

	try {
		initFromConfig(str);
	} catch (...) {	}

	return 0;
}
