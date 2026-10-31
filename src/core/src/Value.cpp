#include "matforge/core/Value.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace matforge::core {

Value::Value() : data_(std::monostate{}) {}

Value::Value(double val) : data_(val) {}

Value::Value(Matrix mat) : data_(std::move(mat)) {}

Value::Value(std::string str) : data_(std::move(str)) {}

Value::Value(const char* str) : data_(std::string(str)) {}

Value::Value(bool b) : data_(b) {}

bool Value::isEmpty() const noexcept {
    return std::holds_alternative<std::monostate>(data_);
}

bool Value::isDouble() const noexcept {
    return std::holds_alternative<double>(data_);
}

bool Value::isMatrix() const noexcept {
    return std::holds_alternative<Matrix>(data_);
}

bool Value::isString() const noexcept {
    return std::holds_alternative<std::string>(data_);
}

bool Value::isBool() const noexcept {
    return std::holds_alternative<bool>(data_);
}

bool Value::isNumeric() const noexcept {
    return isDouble() || isMatrix();
}

double Value::toDouble() const {
    if (isDouble()) {
        return std::get<double>(data_);
    }
    if (isMatrix()) {
        const auto& mat = std::get<Matrix>(data_);
        if (mat.isScalar()) {
            return mat(0, 0);
        }
        throw std::runtime_error("Cannot convert multi-element matrix to scalar double.");
    }
    if (isBool()) {
        return std::get<bool>(data_) ? 1.0 : 0.0;
    }
    throw std::runtime_error("Value is not a numeric type.");
}

Matrix Value::toMatrix() const {
    if (isMatrix()) {
        return std::get<Matrix>(data_);
    }
    if (isDouble()) {
        return Matrix(1, 1, std::get<double>(data_));
    }
    if (isBool()) {
        return Matrix(1, 1, std::get<bool>(data_) ? 1.0 : 0.0);
    }
    throw std::runtime_error("Value cannot be converted to matrix.");
}

const std::string& Value::toStringValue() const {
    if (isString()) {
        return std::get<std::string>(data_);
    }
    throw std::runtime_error("Value is not a string.");
}

bool Value::toBool() const {
    if (isBool()) return std::get<bool>(data_);
    if (isDouble()) return std::get<double>(data_) != 0.0;
    if (isMatrix()) {
        const auto& mat = std::get<Matrix>(data_);
        for (double v : mat.rawData()) {
            if (v == 0.0) return false;
        }
        return !mat.empty();
    }
    return false;
}

std::string Value::typeName() const {
    if (isDouble() || isMatrix()) return "double";
    if (isString()) return "char";
    if (isBool()) return "logical";
    return "void";
}

std::string Value::sizeString() const {
    if (isDouble()) return "1x1";
    if (isMatrix()) {
        const auto& mat = std::get<Matrix>(data_);
        return std::to_string(mat.rows()) + "x" + std::to_string(mat.cols());
    }
    if (isString()) {
        return "1x" + std::to_string(std::get<std::string>(data_).length());
    }
    if (isBool()) return "1x1";
    return "0x0";
}

std::string Value::previewString() const {
    if (isDouble()) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(4) << std::get<double>(data_);
        return ss.str();
    }
    if (isMatrix()) {
        const auto& mat = std::get<Matrix>(data_);
        if (mat.rows() <= 1 && mat.cols() <= 4) {
            std::ostringstream ss;
            ss << "[";
            for (std::size_t i = 0; i < mat.cols(); ++i) {
                ss << mat(0, i) << (i + 1 < mat.cols() ? ", " : "");
            }
            ss << "]";
            return ss.str();
        }
        return "[" + sizeString() + " double]";
    }
    if (isString()) {
        return "'" + std::get<std::string>(data_) + "'";
    }
    if (isBool()) {
        return std::get<bool>(data_) ? "true" : "false";
    }
    return "";
}

std::size_t Value::byteSize() const {
    if (isDouble()) return sizeof(double);
    if (isMatrix()) return sizeof(Matrix) + std::get<Matrix>(data_).size() * sizeof(double);
    if (isString()) return sizeof(std::string) + std::get<std::string>(data_).capacity();
    if (isBool()) return sizeof(bool);
    return 0;
}

std::string Value::formatOutput() const {
    if (isEmpty()) return "";
    if (isDouble()) {
        std::ostringstream ss;
        double v = std::get<double>(data_);
        if (std::abs(v) < 1e-12) v = 0.0;
        ss << "    " << std::fixed << std::setprecision(4) << v;
        return ss.str();
    }
    if (isMatrix()) {
        return std::get<Matrix>(data_).toString();
    }
    if (isString()) {
        return "    " + std::get<std::string>(data_);
    }
    if (isBool()) {
        return std::get<bool>(data_) ? "    logical 1" : "    logical 0";
    }
    return "";
}

Value Value::operator+(const Value& other) const {
    if (isDouble() && other.isDouble()) {
        return Value(toDouble() + other.toDouble());
    }
    if (isMatrix() && other.isDouble()) {
        return Value(toMatrix() + other.toDouble());
    }
    if (isDouble() && other.isMatrix()) {
        return Value(other.toMatrix() + toDouble());
    }
    if (isMatrix() && other.isMatrix()) {
        return Value(toMatrix() + other.toMatrix());
    }
    throw std::runtime_error("Unsupported operand types for addition.");
}

Value Value::operator-(const Value& other) const {
    if (isDouble() && other.isDouble()) {
        return Value(toDouble() - other.toDouble());
    }
    if (isMatrix() && other.isDouble()) {
        return Value(toMatrix() - other.toDouble());
    }
    if (isDouble() && other.isMatrix()) {
        return Value((other.toMatrix() * -1.0) + toDouble());
    }
    if (isMatrix() && other.isMatrix()) {
        return Value(toMatrix() - other.toMatrix());
    }
    throw std::runtime_error("Unsupported operand types for subtraction.");
}

Value Value::operator*(const Value& other) const {
    if (isDouble() && other.isDouble()) {
        return Value(toDouble() * other.toDouble());
    }
    if (isMatrix() && other.isDouble()) {
        return Value(toMatrix() * other.toDouble());
    }
    if (isDouble() && other.isMatrix()) {
        return Value(other.toMatrix() * toDouble());
    }
    if (isMatrix() && other.isMatrix()) {
        return Value(toMatrix() * other.toMatrix());
    }
    throw std::runtime_error("Unsupported operand types for multiplication.");
}

Value Value::operator/(const Value& other) const {
    if (isDouble() && other.isDouble()) {
        double d = other.toDouble();
        if (d == 0.0) throw std::runtime_error("Division by zero.");
        return Value(toDouble() / d);
    }
    if (isMatrix() && other.isDouble()) {
        return Value(toMatrix() / other.toDouble());
    }
    throw std::runtime_error("Unsupported operand types for division.");
}

bool Value::operator==(const Value& other) const {
    if (data_.index() != other.data_.index()) return false;
    if (isDouble()) return std::abs(toDouble() - other.toDouble()) < 1e-9;
    if (isMatrix()) return toMatrix() == other.toMatrix();
    if (isString()) return toStringValue() == other.toStringValue();
    if (isBool()) return toBool() == other.toBool();
    return true;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    os << val.formatOutput();
    return os;
}

} // namespace matforge::core

