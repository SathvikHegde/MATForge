#include "matforge/core/RandomUtils.hpp"

#include <random>
#include <stdexcept>

namespace matforge::core::random {

namespace {
std::mt19937& getRng() {
    static std::mt19937 rng(42);
    return rng;
}
} // namespace

void setSeed(unsigned int seed) {
    getRng().seed(seed);
}

Matrix rand(std::size_t rows, std::size_t cols) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    Matrix m(rows, cols);
    for (double& v : m.rawData()) {
        v = dist(getRng());
    }
    return m;
}

Matrix randn(std::size_t rows, std::size_t cols) {
    std::normal_distribution<double> dist(0.0, 1.0);
    Matrix m(rows, cols);
    for (double& v : m.rawData()) {
        v = dist(getRng());
    }
    return m;
}

Matrix randi(int imax, std::size_t rows, std::size_t cols) {
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

} // namespace matforge::core::random

