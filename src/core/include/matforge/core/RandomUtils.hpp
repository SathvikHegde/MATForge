#pragma once

#include "matforge/core/Matrix.hpp"
#include <cstddef>

namespace matforge::core::random {

void setSeed(unsigned int seed);
Matrix rand(std::size_t rows, std::size_t cols);
Matrix randn(std::size_t rows, std::size_t cols);
Matrix randi(int imax, std::size_t rows, std::size_t cols);

} // namespace matforge::core::random

