#pragma once

#include "matforge/core/Matrix.hpp"

namespace matforge::core::signal {

Matrix conv(const Matrix& u, const Matrix& v);
Matrix fft(const Matrix& x);
Matrix ifft(const Matrix& y);

} // namespace matforge::core::signal

