#pragma once

#include <vector>
#include <string>
#include <initializer_list>
#include <cstddef>
#include <iosfwd>
#include <tuple>
#include <utility>

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
    static Matrix colon(double start, double step, double stop);

    // Slicing and Submatrix access (0-based indices; empty vector indicates all elements ':')
    [[nodiscard]] Matrix submatrix(const std::vector<std::size_t>& rowIndices, 
                                   const std::vector<std::size_t>& colIndices) const;
    [[nodiscard]] Matrix linearSubmatrix(const std::vector<std::size_t>& indices) const;

    // Submatrix assignment (auto-expands matrix if indices exceed bounds)
    void setSubmatrix(const std::vector<std::size_t>& rowIndices, 
                      const std::vector<std::size_t>& colIndices, 
                      const Matrix& val);
    void setLinear(const std::vector<std::size_t>& indices, const Matrix& val);

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

    // Advanced Linear Algebra (Phase 2 - Powered by Eigen)
    [[nodiscard]] Matrix inverse() const;
    [[nodiscard]] Matrix solve(const Matrix& B) const;      // A \ B (mldivide)
    [[nodiscard]] Matrix rightSolve(const Matrix& B) const; // A / B (mrdivide)
    [[nodiscard]] Matrix pinv(double tolerance = -1.0) const;
    [[nodiscard]] std::size_t rank(double tolerance = -1.0) const;
    [[nodiscard]] double cond() const;
    [[nodiscard]] Matrix eigenvalues() const;
    [[nodiscard]] std::pair<Matrix, Matrix> eig() const;    // {V, D}
    [[nodiscard]] Matrix singularValues() const;
    [[nodiscard]] std::tuple<Matrix, Matrix, Matrix> svd() const; // {U, S, V}
    [[nodiscard]] std::pair<Matrix, Matrix> lu() const;     // {L, U}
    [[nodiscard]] std::pair<Matrix, Matrix> qr() const;     // {Q, R}
    [[nodiscard]] Matrix chol() const;                      // Upper Cholesky factor R (R'*R = A)

    // Random Number & Matrix Generation (Phase 2)
    static Matrix rand(std::size_t rows, std::size_t cols);
    static Matrix randn(std::size_t rows, std::size_t cols);
    static Matrix randi(int imax, std::size_t rows, std::size_t cols);
    static void setSeed(unsigned int seed);

    // Statistics (Phase 2)
    [[nodiscard]] double stdDev() const;
    [[nodiscard]] double variance() const;
    [[nodiscard]] double median() const;

    // Signal Processing & Transforms (Phase 2)
    [[nodiscard]] Matrix fft() const;   // Returns N x 2 matrix [real, imag]
    [[nodiscard]] Matrix ifft() const;  // Returns reconstructed signal
    static Matrix conv(const Matrix& u, const Matrix& v); // 1D convolution

    // File I/O (Phase 2)
    static Matrix readCSV(const std::string& path);
    void writeCSV(const std::string& path, char delimiter = ',') const;

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

