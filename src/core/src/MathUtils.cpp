#include "matforge/core/MathUtils.hpp"

#include <cmath>
#include <stdexcept>

namespace matforge::core::math {

Matrix apply(const Matrix& m, double (*func)(double)) {
    Matrix res(m.rows(), m.cols());
    for (std::size_t i = 0; i < m.size(); ++i) {
        res.rawData()[i] = func(m.rawData()[i]);
    }
    return res;
}

static Value applyFunction(const Value& val, double (*func)(double)) {
    if (val.isDouble()) {
        return Value(func(val.toDouble()));
    }
    if (val.isMatrix()) {
        return Value(apply(val.toMatrix(), func));
    }
    throw std::runtime_error("Mathematical function can only be applied to numeric values.");
}

Value sin(const Value& val) {
    return applyFunction(val, std::sin);
}

Value cos(const Value& val) {
    return applyFunction(val, std::cos);
}

Value tan(const Value& val) {
    return applyFunction(val, std::tan);
}

Value sqrt(const Value& val) {
    return applyFunction(val, std::sqrt);
}

Value exp(const Value& val) {
    return applyFunction(val, std::exp);
}

Value log(const Value& val) {
    return applyFunction(val, std::log);
}

Value abs(const Value& val) {
    return applyFunction(val, std::abs);
}

} // namespace matforge::core::math

