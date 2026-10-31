#pragma once

#include "matforge/core/Matrix.hpp"
#include <variant>
#include <string>
#include <cstddef>
#include <iosfwd>

namespace matforge::core {

class Value {
public:
    using VariantType = std::variant<std::monostate, double, Matrix, std::string, bool>;

    // Constructors
    Value();
    Value(double val);
    Value(Matrix mat);
    Value(std::string str);
    Value(const char* str);
    Value(bool b);

    // Type queries
    [[nodiscard]] bool isEmpty() const noexcept;
    [[nodiscard]] bool isDouble() const noexcept;
    [[nodiscard]] bool isMatrix() const noexcept;
    [[nodiscard]] bool isString() const noexcept;
    [[nodiscard]] bool isBool() const noexcept;
    [[nodiscard]] bool isNumeric() const noexcept;

    // Conversions
    [[nodiscard]] double toDouble() const;
    [[nodiscard]] Matrix toMatrix() const;
    [[nodiscard]] const std::string& toStringValue() const;
    [[nodiscard]] bool toBool() const;

    // Metadata for Workspace viewer
    [[nodiscard]] std::string typeName() const;
    [[nodiscard]] std::string sizeString() const;
    [[nodiscard]] std::string previewString() const;
    [[nodiscard]] std::size_t byteSize() const;

    // Formatting as MATLAB terminal output
    [[nodiscard]] std::string formatOutput() const;

    // Operators
    [[nodiscard]] Value operator+(const Value& other) const;
    [[nodiscard]] Value operator-(const Value& other) const;
    [[nodiscard]] Value operator*(const Value& other) const;
    [[nodiscard]] Value operator/(const Value& other) const;

    [[nodiscard]] bool operator==(const Value& other) const;
    [[nodiscard]] bool operator!=(const Value& other) const;

private:
    VariantType data_;
};

std::ostream& operator<<(std::ostream& os, const Value& val);

} // namespace matforge::core

