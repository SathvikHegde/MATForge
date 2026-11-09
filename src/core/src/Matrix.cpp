#include "matforge/core/Matrix.hpp"

#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <random>
#include <algorithm>
#include <numeric>
#include <limits>

// Eigen 5 headers
#include <Eigen/Dense>
#include <Eigen/LU>
#include <Eigen/QR>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <Eigen/Cholesky>

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

Matrix Matrix::colon(double start, double step, double stop) {
    if (step == 0.0) {
        throw std::invalid_argument("Colon step size cannot be zero.");
    }
    if ((step > 0.0 && start > stop) || (step < 0.0 && start < stop)) {
        return Matrix(1, 0);
    }

    std::size_t num = static_cast<std::size_t>(std::floor((stop - start) / step + 1e-10)) + 1;
    Matrix result(1, num);
    for (std::size_t i = 0; i < num; ++i) {
        result(0, i) = start + static_cast<double>(i) * step;
    }
    return result;
}

Matrix Matrix::submatrix(const std::vector<std::size_t>& rowIndices, 
                         const std::vector<std::size_t>& colIndices) const {
    std::vector<std::size_t> actualRows = rowIndices;
    if (actualRows.empty()) {
        actualRows.resize(rows_);
        for (std::size_t i = 0; i < rows_; ++i) actualRows[i] = i;
    }

    std::vector<std::size_t> actualCols = colIndices;
    if (actualCols.empty()) {
        actualCols.resize(cols_);
        for (std::size_t i = 0; i < cols_; ++i) actualCols[i] = i;
    }

    for (std::size_t r : actualRows) {
        if (r >= rows_) throw std::out_of_range("Row index out of matrix bounds.");
    }
    for (std::size_t c : actualCols) {
        if (c >= cols_) throw std::out_of_range("Column index out of matrix bounds.");
    }

    Matrix result(actualRows.size(), actualCols.size());
    for (std::size_t r = 0; r < actualRows.size(); ++r) {
        for (std::size_t c = 0; c < actualCols.size(); ++c) {
            result(r, c) = (*this)(actualRows[r], actualCols[c]);
        }
    }
    return result;
}

Matrix Matrix::linearSubmatrix(const std::vector<std::size_t>& indices) const {
    for (std::size_t idx : indices) {
        if (idx >= size()) throw std::out_of_range("Linear index exceeds matrix bounds.");
    }

    bool asRow = (rows_ == 1);
    std::size_t outRows = asRow ? 1 : indices.size();
    std::size_t outCols = asRow ? indices.size() : 1;
    Matrix result(outRows, outCols);

    for (std::size_t i = 0; i < indices.size(); ++i) {
        result.rawData()[i] = data_[indices[i]];
    }
    return result;
}

void Matrix::setSubmatrix(const std::vector<std::size_t>& rowIndices, 
                          const std::vector<std::size_t>& colIndices, 
                          const Matrix& val) {
    std::size_t maxR = rows_ > 0 ? rows_ - 1 : 0;
    for (std::size_t r : rowIndices) {
        if (r > maxR) maxR = r;
    }

    std::size_t maxC = cols_ > 0 ? cols_ - 1 : 0;
    for (std::size_t c : colIndices) {
        if (c > maxC) maxC = c;
    }

    std::size_t newRows = std::max(rows_, maxR + 1);
    std::size_t newCols = std::max(cols_, maxC + 1);

    if (newRows > rows_ || newCols > cols_) {
        std::vector<double> newData(newRows * newCols, 0.0);
        for (std::size_t r = 0; r < rows_; ++r) {
            for (std::size_t c = 0; c < cols_; ++c) {
                newData[r * newCols + c] = (*this)(r, c);
            }
        }
        rows_ = newRows;
        cols_ = newCols;
        data_ = std::move(newData);
    }

    std::vector<std::size_t> actualRows = rowIndices;
    if (actualRows.empty()) {
        actualRows.resize(rows_);
        for (std::size_t i = 0; i < rows_; ++i) actualRows[i] = i;
    }

    std::vector<std::size_t> actualCols = colIndices;
    if (actualCols.empty()) {
        actualCols.resize(cols_);
        for (std::size_t i = 0; i < cols_; ++i) actualCols[i] = i;
    }

    if (val.isScalar()) {
        double s = val.rawData()[0];
        for (std::size_t r : actualRows) {
            for (std::size_t c : actualCols) {
                (*this)(r, c) = s;
            }
        }
    } else {
        if (actualRows.size() != val.rows() || actualCols.size() != val.cols()) {
            if (actualRows.size() * actualCols.size() == val.size()) {
                // Compatible 1D linear mapping
                std::size_t valIdx = 0;
                for (std::size_t r : actualRows) {
                    for (std::size_t c : actualCols) {
                        (*this)(r, c) = val.rawData()[valIdx++];
                    }
                }
                return;
            }
            throw std::invalid_argument("Submatrix assignment dimension mismatch.");
        }
        for (std::size_t r = 0; r < actualRows.size(); ++r) {
            for (std::size_t c = 0; c < actualCols.size(); ++c) {
                (*this)(actualRows[r], actualCols[c]) = val(r, c);
            }
        }
    }
}

void Matrix::setLinear(const std::vector<std::size_t>& indices, const Matrix& val) {
    if (indices.empty()) return;

    std::size_t maxIdx = *std::max_element(indices.begin(), indices.end());
    if (maxIdx >= size()) {
        std::size_t newSize = maxIdx + 1;
        if (rows_ <= 1) {
            rows_ = 1;
            cols_ = newSize;
        } else {
            rows_ = (newSize + cols_ - 1) / cols_;
            newSize = rows_ * cols_;
        }
        data_.resize(newSize, 0.0);
    }

    if (val.isScalar()) {
        double s = val.rawData()[0];
        for (std::size_t idx : indices) {
            data_[idx] = s;
        }
    } else {
        if (indices.size() != val.size()) {
            throw std::invalid_argument("Linear assignment length mismatch.");
        }
        for (std::size_t i = 0; i < indices.size(); ++i) {
            data_[indices[i]] = val.rawData()[i];
        }
    }
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
    if (empty()) return 1.0;
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);
    return mapA.determinant();
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

// =========================================================================
// Advanced Linear Algebra (Phase 2 - Powered by Eigen 5)
// =========================================================================

Matrix Matrix::inverse() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute inverse.");
    }
    if (empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::FullPivLU<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> lu(mapA);
    if (!lu.isInvertible()) {
        throw std::runtime_error("Matrix is singular to working precision.");
    }
    Matrix result(rows_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapRes(result.rawData().data(), rows_, cols_);
    mapRes = lu.inverse();
    return result;
}

Matrix Matrix::solve(const Matrix& B) const {
    if (rows_ != B.rows_) {
        throw std::invalid_argument("Matrix row dimensions must agree for solve (A \\ B).");
    }
    if (empty() || B.empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapB(B.rawData().data(), B.rows(), B.cols());

    Matrix result(cols_, B.cols());
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapX(result.rawData().data(), cols_, B.cols());

    mapX = mapA.colPivHouseholderQr().solve(mapB);
    return result;
}

Matrix Matrix::rightSolve(const Matrix& B) const {
    // A / B = (B' \ A')'
    return (B.transpose().solve(this->transpose())).transpose();
}

Matrix Matrix::pinv(double tolerance) const {
    if (empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mapA, Eigen::ComputeThinU | Eigen::ComputeThinV);
    const auto& s = svd.singularValues();
    double tol = tolerance;
    if (tol <= 0.0) {
        tol = static_cast<double>(std::max(rows_, cols_)) * s(0) * std::numeric_limits<double>::epsilon();
    }

    Eigen::VectorXd sInv = Eigen::VectorXd::Zero(s.size());
    for (Eigen::Index i = 0; i < s.size(); ++i) {
        if (s(i) > tol) {
            sInv(i) = 1.0 / s(i);
        }
    }

    Eigen::MatrixXd pinvMat = svd.matrixV() * sInv.asDiagonal() * svd.matrixU().adjoint();
    Matrix result(cols_, rows_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapRes(result.rawData().data(), cols_, rows_);
    mapRes = pinvMat;
    return result;
}

std::size_t Matrix::rank(double tolerance) const {
    if (empty()) return 0;
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mapA);
    const auto& s = svd.singularValues();
    double tol = tolerance;
    if (tol <= 0.0) {
        tol = static_cast<double>(std::max(rows_, cols_)) * s(0) * std::numeric_limits<double>::epsilon();
    }
    std::size_t r = 0;
    for (Eigen::Index i = 0; i < s.size(); ++i) {
        if (s(i) > tol) ++r;
    }
    return r;
}

double Matrix::cond() const {
    if (empty()) return 0.0;
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mapA);
    const auto& s = svd.singularValues();
    if (s.size() == 0 || s(s.size() - 1) == 0.0) {
        return std::numeric_limits<double>::infinity();
    }
    return s(0) / s(s.size() - 1);
}

Matrix Matrix::eigenvalues() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute eigenvalues.");
    }
    if (empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::EigenSolver<Eigen::MatrixXd> es(mapA);
    auto e = es.eigenvalues();
    bool hasImag = false;
    for (Eigen::Index i = 0; i < e.size(); ++i) {
        if (std::abs(e(i).imag()) > 1e-10) {
            hasImag = true;
            break;
        }
    }
    if (!hasImag) {
        Matrix result(static_cast<std::size_t>(e.size()), 1);
        for (Eigen::Index i = 0; i < e.size(); ++i) {
            result(static_cast<std::size_t>(i), 0) = e(i).real();
        }
        return result;
    } else {
        Matrix result(static_cast<std::size_t>(e.size()), 2);
        for (Eigen::Index i = 0; i < e.size(); ++i) {
            result(static_cast<std::size_t>(i), 0) = e(i).real();
            result(static_cast<std::size_t>(i), 1) = e(i).imag();
        }
        return result;
    }
}

std::pair<Matrix, Matrix> Matrix::eig() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute eig.");
    }
    if (empty()) return {Matrix(), Matrix()};
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::EigenSolver<Eigen::MatrixXd> es(mapA);
    auto e = es.eigenvalues();
    auto ev = es.eigenvectors();

    Matrix V(rows_, cols_);
    Matrix D = Matrix::zeros(rows_, cols_);
    for (std::size_t i = 0; i < rows_; ++i) {
        D(i, i) = e(static_cast<Eigen::Index>(i)).real();
        for (std::size_t j = 0; j < cols_; ++j) {
            V(i, j) = ev(static_cast<Eigen::Index>(i), static_cast<Eigen::Index>(j)).real();
        }
    }
    return {V, D};
}

Matrix Matrix::singularValues() const {
    if (empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mapA);
    const auto& s = svd.singularValues();
    Matrix result(static_cast<std::size_t>(s.size()), 1);
    for (Eigen::Index i = 0; i < s.size(); ++i) {
        result(static_cast<std::size_t>(i), 0) = s(i);
    }
    return result;
}

std::tuple<Matrix, Matrix, Matrix> Matrix::svd() const {
    if (empty()) return {Matrix(), Matrix(), Matrix()};
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mapA, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Matrix U(rows_, rows_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapU(U.rawData().data(), rows_, rows_);
    mapU = svd.matrixU();

    Matrix S = Matrix::zeros(rows_, cols_);
    const auto& s = svd.singularValues();
    for (Eigen::Index i = 0; i < s.size(); ++i) {
        S(static_cast<std::size_t>(i), static_cast<std::size_t>(i)) = s(i);
    }

    Matrix V(cols_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapV(V.rawData().data(), cols_, cols_);
    mapV = svd.matrixV();

    return {U, S, V};
}

std::pair<Matrix, Matrix> Matrix::lu() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square to compute LU.");
    }
    if (empty()) return {Matrix(), Matrix()};
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::PartialPivLU<Eigen::MatrixXd> lu(mapA);
    Eigen::MatrixXd L = lu.matrixLU().triangularView<Eigen::StrictlyLower>();
    L += Eigen::MatrixXd::Identity(rows_, cols_);
    L = lu.permutationP().inverse() * L;
    Eigen::MatrixXd U = lu.matrixLU().triangularView<Eigen::Upper>();

    Matrix matL(rows_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapL(matL.rawData().data(), rows_, cols_);
    mapL = L;

    Matrix matU(rows_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapU(matU.rawData().data(), rows_, cols_);
    mapU = U;

    return {matL, matU};
}

std::pair<Matrix, Matrix> Matrix::qr() const {
    if (empty()) return {Matrix(), Matrix()};
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::HouseholderQR<Eigen::MatrixXd> qr(mapA);
    Eigen::MatrixXd Q = qr.householderQ() * Eigen::MatrixXd::Identity(rows_, rows_);
    Eigen::MatrixXd R = qr.matrixQR().triangularView<Eigen::Upper>();
    R.conservativeResize(rows_, cols_);

    Matrix matQ(rows_, rows_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapQ(matQ.rawData().data(), rows_, rows_);
    mapQ = Q;

    Matrix matR(rows_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapR(matR.rawData().data(), rows_, cols_);
    mapR = R;

    return {matQ, matR};
}

Matrix Matrix::chol() const {
    if (!isSquare()) {
        throw std::invalid_argument("Matrix must be square for Cholesky decomposition.");
    }
    if (empty()) return Matrix();
    Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapA(data_.data(), rows_, cols_);

    Eigen::LLT<Eigen::MatrixXd> llt(mapA);
    if (llt.info() != Eigen::Success) {
        throw std::runtime_error("Matrix must be positive definite for Cholesky decomposition.");
    }
    Eigen::MatrixXd U = llt.matrixU();
    Matrix result(rows_, cols_);
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>
        mapRes(result.rawData().data(), rows_, cols_);
    mapRes = U;
    return result;
}

// =========================================================================
// Random Numbers & Statistics (Phase 2)
// =========================================================================

namespace {
std::mt19937_64& getRng() {
    static std::mt19937_64 rng(1337);
    return rng;
}
} // namespace

void Matrix::setSeed(unsigned int seed) {
    getRng().seed(seed);
}

Matrix Matrix::rand(std::size_t rows, std::size_t cols) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    Matrix m(rows, cols);
    for (double& v : m.rawData()) {
        v = dist(getRng());
    }
    return m;
}

Matrix Matrix::randn(std::size_t rows, std::size_t cols) {
    std::normal_distribution<double> dist(0.0, 1.0);
    Matrix m(rows, cols);
    for (double& v : m.rawData()) {
        v = dist(getRng());
    }
    return m;
}

Matrix Matrix::randi(int imax, std::size_t rows, std::size_t cols) {
    if (imax < 1) {
        throw std::invalid_argument("imax must be >= 1 for randi.");
    }
    std::uniform_int_distribution<int> dist(1, imax);
    Matrix m(rows, cols);
    for (double& v : m.rawData()) {
        v = static_cast<double>(dist(getRng()));
    }
    return m;
}

double Matrix::variance() const {
    if (data_.size() <= 1) return 0.0;
    double m = mean();
    double sumSq = 0.0;
    for (double v : data_) {
        double diff = v - m;
        sumSq += diff * diff;
    }
    return sumSq / static_cast<double>(data_.size() - 1);
}

double Matrix::stdDev() const {
    return std::sqrt(variance());
}

double Matrix::median() const {
    if (data_.empty()) {
        throw std::runtime_error("Cannot compute median of empty matrix.");
    }
    std::vector<double> sorted = data_;
    std::sort(sorted.begin(), sorted.end());
    std::size_t n = sorted.size();
    if (n % 2 == 1) {
        return sorted[n / 2];
    }
    return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
}

// =========================================================================
// Signal Processing & Transforms (Phase 2)
// =========================================================================

Matrix Matrix::conv(const Matrix& u, const Matrix& v) {
    if (!u.isVector() || !v.isVector()) {
        throw std::invalid_argument("conv arguments must be vectors.");
    }
    if (u.empty() || v.empty()) {
        return Matrix();
    }
    std::size_t n = u.size() + v.size() - 1;
    std::vector<double> out(n, 0.0);
    const auto& uData = u.rawData();
    const auto& vData = v.rawData();
    for (std::size_t i = 0; i < uData.size(); ++i) {
        for (std::size_t j = 0; j < vData.size(); ++j) {
            out[i + j] += uData[i] * vData[j];
        }
    }
    if (u.rows() == 1) {
        return Matrix(1, n, out);
    } else {
        return Matrix(n, 1, out);
    }
}

namespace {
void cooleyTukeyFFT(std::vector<std::complex<double>>& a, bool inverse) {
    std::size_t n = a.size();
    if (n <= 1) return;

    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    const double pi = 3.14159265358979323846;
    for (std::size_t len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * pi / static_cast<double>(len) * (inverse ? 1.0 : -1.0);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t j = 0; j < len / 2; ++j) {
                std::complex<double> u = a[i + j];
                std::complex<double> v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse) {
        for (std::size_t i = 0; i < n; ++i) {
            a[i] /= static_cast<double>(n);
        }
    }
}

void directDFT(const std::vector<std::complex<double>>& in, std::vector<std::complex<double>>& out, bool inverse) {
    std::size_t n = in.size();
    out.assign(n, std::complex<double>(0.0, 0.0));
    const double pi = 3.14159265358979323846;
    for (std::size_t k = 0; k < n; ++k) {
        for (std::size_t j = 0; j < n; ++j) {
            double angle = 2.0 * pi * static_cast<double>(k * j) / static_cast<double>(n) * (inverse ? 1.0 : -1.0);
            std::complex<double> twiddle(std::cos(angle), std::sin(angle));
            out[k] += in[j] * twiddle;
        }
        if (inverse) {
            out[k] /= static_cast<double>(n);
        }
    }
}

void computeFFT(std::vector<std::complex<double>>& data, bool inverse) {
    std::size_t n = data.size();
    if (n <= 1) return;
    bool isPowerOfTwo = (n & (n - 1)) == 0;
    if (isPowerOfTwo) {
        cooleyTukeyFFT(data, inverse);
    } else {
        std::vector<std::complex<double>> out;
        directDFT(data, out, inverse);
        data = std::move(out);
    }
}
} // namespace

Matrix Matrix::fft() const {
    if (empty()) return Matrix();
    std::size_t n = size();
    std::vector<std::complex<double>> cdata(n);
    if (cols_ == 2 && rows_ > 1) {
        for (std::size_t i = 0; i < rows_; ++i) {
            cdata[i] = std::complex<double>((*this)(i, 0), (*this)(i, 1));
        }
        n = rows_;
        cdata.resize(n);
    } else {
        for (std::size_t i = 0; i < n; ++i) {
            cdata[i] = std::complex<double>(data_[i], 0.0);
        }
    }

    computeFFT(cdata, false);

    Matrix result(n, 2);
    for (std::size_t i = 0; i < n; ++i) {
        result(i, 0) = cdata[i].real();
        result(i, 1) = cdata[i].imag();
    }
    return result;
}

Matrix Matrix::ifft() const {
    if (empty()) return Matrix();
    std::size_t n = rows_;
    std::vector<std::complex<double>> cdata;

    if (cols_ == 2) {
        cdata.resize(rows_);
        for (std::size_t i = 0; i < rows_; ++i) {
            cdata[i] = std::complex<double>((*this)(i, 0), (*this)(i, 1));
        }
    } else {
        n = size();
        cdata.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            cdata[i] = std::complex<double>(data_[i], 0.0);
        }
    }

    computeFFT(cdata, true);

    bool hasImag = false;
    for (const auto& c : cdata) {
        if (std::abs(c.imag()) > 1e-9) {
            hasImag = true;
            break;
        }
    }

    if (!hasImag) {
        Matrix result(n, 1);
        for (std::size_t i = 0; i < n; ++i) {
            result(i, 0) = cdata[i].real();
        }
        return result;
    } else {
        Matrix result(n, 2);
        for (std::size_t i = 0; i < n; ++i) {
            result(i, 0) = cdata[i].real();
            result(i, 1) = cdata[i].imag();
        }
        return result;
    }
}

// =========================================================================
// File I/O (Phase 2)
// =========================================================================

Matrix Matrix::readCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + path);
    }
    std::vector<std::vector<double>> rows;
    std::string line;
    while (std::getline(file, line)) {
        auto start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        if (line[start] == '#' || line[start] == '%') continue;

        for (char& ch : line) {
            if (ch == ',' || ch == ';' || ch == '\t') ch = ' ';
        }
        std::stringstream ss(line);
        std::vector<double> row;
        double val;
        while (ss >> val) {
            row.push_back(val);
        }
        if (!row.empty()) {
            rows.push_back(std::move(row));
        }
    }
    if (rows.empty()) {
        return Matrix();
    }
    std::size_t numRows = rows.size();
    std::size_t numCols = rows[0].size();
    std::vector<double> flattened;
    flattened.reserve(numRows * numCols);
    for (const auto& r : rows) {
        if (r.size() != numCols) {
            throw std::runtime_error("Inconsistent column count in CSV file.");
        }
        for (double d : r) {
            flattened.push_back(d);
        }
    }
    return Matrix(numRows, numCols, flattened);
}

void Matrix::writeCSV(const std::string& path, char delimiter) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + path);
    }
    file << std::fixed << std::setprecision(10);
    for (std::size_t r = 0; r < rows_; ++r) {
        for (std::size_t c = 0; c < cols_; ++c) {
            file << (*this)(r, c);
            if (c + 1 < cols_) {
                file << delimiter;
            }
        }
        file << "\n";
    }
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

