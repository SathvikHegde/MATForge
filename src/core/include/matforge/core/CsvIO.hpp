#pragma once

#include "matforge/core/Matrix.hpp"
#include <string>

namespace matforge::core::io {

Matrix readCSV(const std::string& path);
void writeCSV(const Matrix& mat, const std::string& path, char delimiter = ',');

} // namespace matforge::core::io

