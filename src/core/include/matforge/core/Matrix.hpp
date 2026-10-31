#pragma once

#include <vector>
#include <string>
#include <initializer_list>
#include <cstddef>
#include <iosfwd>

namespace matforge::core {

class Matrix {
public:
    // Constructors
    Matrix();
    Matrix(std::size_t rows, std::size_t cols, double initialValue = 0.0);
    Matrix(std::size_t rows, std::size_t cols, const std::vector<double>& data);
    Matrix(std::initializer_list<std::initializer_list<double>> list);

    // Dimension inspection
    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t cols() const noexcept { return cols_; }
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] bool isScalar() const noexcept { return rows_ == 1 && cols_ == 1; }
    [[nodiscard]] bool isVector() const noexcept { return rows_ == 1 || cols_ == 1; }
    [[nodiscard]] bool isSquare() const noexcept { return rows_ == cols_ && rows_ > 0; }

    // Element access (0-based internally)
    [[nodiscard]] double& operator()(std::size_t row, std::size_t col);
    [[nodiscard]] const double& operator()(std::size_t row, std::size_t col) const;
    [[nodiscard]] double& at(std::size_t row, std::size_t col);
    [[nodiscard]] const double& at(std::size_t row, std::size_t col) const;

    // Direct data access
    [[nodiscard]] const std::vector<double>& rawData() const noexcept { return data_; }
    [[nodiscard]] std::vector<double>& rawData() noexcept { return data_; }

    // Static Factory Helpers
    static Matrix zeros(std::size_t rows, std::size_t cols);
    static Matrix ones(std::size_t rows, std::size_t cols);
    static Matrix eye(std::size_t n);
    static Matrix eye(std::size_t rows, std::size_t cols);
    static Matrix linspace(double start, double stop, std::size_t num = 100);

    // Matrix Operations
    [[nodiscard]] Matrix transpose() const;
    [[nodiscard]] Matrix operator+(const Matrix& other) const;
    [[nodiscard]] Matrix operator-(const Matrix& other) const;
    [[nodiscard]] Matrix operator*(const Matrix& other) const;
    Matrix& operator+=(const Matrix& other);
    Matrix& operator-=(const Matrix& other);

    // Scalar Operations
    [[nodiscard]] Matrix operator+(double scalar) const;
    [[nodiscard]] Matrix operator-(double scalar) const;
    [[nodiscard]] Matrix operator*(double scalar) const;
    [[nodiscard]] Matrix operator/(double scalar) const;
    friend Matrix operator+(double scalar, const Matrix& mat);
    friend Matrix operator*(double scalar, const Matrix& mat);

    // Element-wise operations (MATLAB .* and ./)
    [[nodiscard]] Matrix elementwiseMultiply(const Matrix& other) const;
    [[nodiscard]] Matrix elementwiseDivide(const Matrix& other) const;
    [[nodiscard]] Matrix elementwisePower(double exponent) const;

    // Linear Algebra & Aggregations
    [[nodiscard]] double determinant() const;
    [[nodiscard]] double trace() const;
    [[nodiscard]] double sum() const;
    [[nodiscard]] double mean() const;
    [[nodiscard]] double min() const;
    [[nodiscard]] double max() const;

    // Reshaping
    void reshape(std::size_t newRows, std::size_t newCols);

    // Comparisons
    bool operator==(const Matrix& other) const;
    bool operator!=(const Matrix& other) const;

    // Formatting & Display (MATLAB output style)
    [[nodiscard]] std::string toString(int precision = 4) const;

private:
    std::size_t rows_{0};
    std::size_t cols_{0};
    std::vector<double> data_;

    // Helper for computing determinant of submatrix
    double determinantRecursive(const std::vector<double>& mat, std::size_t n) const;
};

std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

} // namespace matforge::core

