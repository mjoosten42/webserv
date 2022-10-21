#pragma once

#include <string>

namespace MIME {

std::string fromFileName(const std::string& filename);
std::string getExtension(const std::string& filename);

} // namespace MIME
