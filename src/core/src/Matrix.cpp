#include "matforge/core/Matrix.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace matforge::core {

Matrix::Matrix() : rows_(0), cols_(0) {}

Matrix::Matrix(std::size_t rows, std::size_t cols, double initialValue)
    : rows_(rows), cols_(cols), data_(rows * cols, initialValue) {}

Matrix::Matrix(std::size_t rows, std::size_t cols, const std::vector<double>& data)
    : rows_(rows), cols_(cols), data_(data) {
    if (data.size() != rows * cols) {
        throw std::invalid_argument("Matrix dimension does not match data size.");
    }
}

Matrix::Matrix(std::initializer_list<std::initializer_list<double>> list) {
    rows_ = list.size();
    cols_ = rows_ > 0 ? list.begin()->size() : 0;
    data_.reserve(rows_ * cols_);

    for (const auto& row : list) {
        if (row.size() != cols_) {
            throw std::invalid_argument("All rows in matrix initializer list must have equal length.");
        }
        data_.insert(data_.end(), row.begin(), row.end());
    }
}

double& Matrix::operator()(std::size_t row, std::size_t col) {
    return data_[row * cols_ + col];
}

const double& Matrix::operator()(std::size_t row, std::size_t col) const {
    return data_[row * cols_ + col];
}

double& Matrix::at(std::size_t row, std::size_t col) {
    if (row >= rows_ || col >= cols_) {
        throw std::out_of_range("Matrix index out of bounds.");
    }
    return data_[row * cols_ + col];
}

const double& Matrix::at(std::size_t row, std::size_t col) const {
    if (row >= rows_ || col >= cols_) {
        throw std::out_of_range("Matrix index out of bounds.");
    }
    return data_[row * cols_ + col];
}

Matrix Matrix::zeros(std::size_t rows, std::size_t cols) {
    return Matrix(rows, cols, 0.0);
}

Matrix Matrix::ones(std::size_t rows, std::size_t cols) {
    return Matrix(rows, cols, 1.0);
}

Matrix Matrix::eye(std::size_t n) {
    return eye(n, n);
}

Matrix Matrix::eye(std::size_t rows, std::size_t cols) {
    Matrix m(rows, cols, 0.0);
    std::size_t minDim = std::min(rows, cols);
    for (std::size_t i = 0; i < minDim; ++i) {
        m(i, i) = 1.0;
    }
    return m;
}

Matrix Matrix::linspace(double start, double stop, std::size_t num) {
    if (num == 0) {
        return Matrix(0, 0);
    }
    if (num == 1) {
        return Matrix(1, 1, start);
    }

    Matrix result(1, num);
    double step = (stop - start) / static_cast<double>(num - 1);
    for (std::size_t i = 0; i < num; ++i) {
        result(0, i) = start + static_cast<double>(i) * step;
    }
    return result;
}

Matrix Matrix::transpose() const {
    Matrix result(cols_, rows_);
    for (std::size_t r = 0; r < rows_; ++r) {
        for (std::size_t c = 0; c < cols_; ++c) {
            result(c, r) = (*this)(r, c);
        }
    }
    return result;
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must agree for addition.");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must agree for subtraction.");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (cols_ != other.rows_) {
        throw std::invalid_argument("Matrix inner dimensions must agree for multiplication.");
    }
    Matrix result(rows_, other.cols_, 0.0);
    for (std::size_t i = 0; i < rows_; ++i) {
        for (std::size_t k = 0; k < cols_; ++k) {
            double elemA = (*this)(i, k);
            for (std::size_t j = 0; j < other.cols_; ++j) {
                result(i, j) += elemA * other(k, j);
            }
        }
    }
    return result;
}

Matrix& Matrix::operator+=(const Matrix& other) {
    *this = *this + other;
    return *this;
}

Matrix& Matrix::operator-=(const Matrix& other) {
    *this = *this - other;
    return *this;
}

Matrix Matrix::operator+(double scalar) const {
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + scalar;
    }
    return result;
}

Matrix Matrix::operator-(double scalar) const {
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - scalar;
    }
    return result;
}

Matrix Matrix::operator*(double scalar) const {
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

Matrix Matrix::operator/(double scalar) const {
    if (scalar == 0.0) {
        throw std::runtime_error("Division by zero.");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] / scalar;
    }
    return result;
}

Matrix operator+(double scalar, const Matrix& mat) {
    return mat + scalar;
}

Matrix operator*(double scalar, const Matrix& mat) {
    return mat * scalar;
}

Matrix Matrix::elementwiseMultiply(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must agree for element-wise multiplication.");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * other.data_[i];
    }
    return result;
}

Matrix Matrix::elementwiseDivide(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must agree for element-wise division.");
    }
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] / other.data_[i];
    }
    return result;
}

Matrix Matrix::elementwisePower(double exponent) const {
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = std::pow(data_[i], exponent);
    }
    return result;
}

double Matrix::determinant() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute determinant.");
    }
    return determinantRecursive(data_, rows_);
}

double Matrix::determinantRecursive(const std::vector<double>& mat, std::size_t n) const {
    if (n == 1) {
        return mat[0];
    }
    if (n == 2) {
        return mat[0] * mat[3] - mat[1] * mat[2];
    }

    double det = 0.0;
    std::vector<double> submat((n - 1) * (n - 1));
    int sign = 1;

    for (std::size_t c = 0; c < n; ++c) {
        std::size_t subi = 0;
        for (std::size_t i = 1; i < n; ++i) {
            std::size_t subj = 0;
            for (std::size_t j = 0; j < n; ++j) {
                if (j == c) continue;
                submat[subi * (n - 1) + subj] = mat[i * n + j];
                subj++;
            }
            subi++;
        }
        det += sign * mat[c] * determinantRecursive(submat, n - 1);
        sign = -sign;
    }
    return det;
}

double Matrix::trace() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute trace.");
    }
    double tr = 0.0;
    for (std::size_t i = 0; i < rows_; ++i) {
        tr += (*this)(i, i);
    }
    return tr;
}

double Matrix::sum() const {
    return std::accumulate(data_.begin(), data_.end(), 0.0);
}

double Matrix::mean() const {
    if (data_.empty()) return 0.0;
    return sum() / static_cast<double>(data_.size());
}

double Matrix::min() const {
    if (data_.empty()) throw std::runtime_error("Cannot find min of empty matrix.");
    return *std::min_element(data_.begin(), data_.end());
}

double Matrix::max() const {
    if (data_.empty()) throw std::runtime_error("Cannot find max of empty matrix.");
    return *std::max_element(data_.begin(), data_.end());
}

void Matrix::reshape(std::size_t newRows, std::size_t newCols) {
    if (newRows * newCols != data_.size()) {
        throw std::invalid_argument("Total number of elements must not change during reshape.");
    }
    rows_ = newRows;
    cols_ = newCols;
}

bool Matrix::operator==(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        return false;
    }
    for (std::size_t i = 0; i < data_.size(); ++i) {
        if (std::abs(data_[i] - other.data_[i]) > 1e-9) {
            return false;
        }
    }
    return true;
}

bool Matrix::operator!=(const Matrix& other) const {
    return !(*this == other);
}

std::string Matrix::toString(int precision) const {
    if (empty()) {
        return "[]";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision);

    for (std::size_t r = 0; r < rows_; ++r) {
        oss << "   ";
        for (std::size_t c = 0; c < cols_; ++c) {
            double val = (*this)(r, c);
            // Treat very tiny numbers as 0
            if (std::abs(val) < 1e-12) val = 0.0;
            oss << std::setw(precision + 6) << val;
        }
        if (r + 1 < rows_) {
            oss << "\n";
        }
    }
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
    os << matrix.toString();
    return os;
}

} // namespace matforge::core

