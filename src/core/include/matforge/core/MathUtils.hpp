#pragma once

#include "matforge/core/Matrix.hpp"
#include "matforge/core/Value.hpp"

namespace matforge::core::math {

Value sin(const Value& val);
Value cos(const Value& val);
Value tan(const Value& val);
Value sqrt(const Value& val);
Value exp(const Value& val);
Value log(const Value& val);
Value abs(const Value& val);

Matrix apply(const Matrix& m, double (*func)(double));

} // namespace matforge::core::math

