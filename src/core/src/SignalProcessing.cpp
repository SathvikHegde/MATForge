#include "matforge/core/SignalProcessing.hpp"

#include <complex>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace matforge::core::signal {

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

Matrix conv(const Matrix& u, const Matrix& v) {
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

Matrix fft(const Matrix& x) {
    if (x.empty()) return Matrix();
    std::size_t n = x.size();
    std::vector<std::complex<double>> cdata(n);
    if (x.cols() == 2 && x.rows() > 1) {
        for (std::size_t i = 0; i < x.rows(); ++i) {
            cdata[i] = std::complex<double>(x(i, 0), x(i, 1));
        }
        n = x.rows();
        cdata.resize(n);
    } else {
        for (std::size_t i = 0; i < n; ++i) {
            cdata[i] = std::complex<double>(x.rawData()[i], 0.0);
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

Matrix ifft(const Matrix& y) {
    if (y.empty()) return Matrix();
    std::size_t n = y.rows();
    std::vector<std::complex<double>> cdata;

    if (y.cols() == 2) {
        cdata.resize(y.rows());
        for (std::size_t i = 0; i < y.rows(); ++i) {
            cdata[i] = std::complex<double>(y(i, 0), y(i, 1));
        }
    } else {
        n = y.size();
        cdata.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            cdata[i] = std::complex<double>(y.rawData()[i], 0.0);
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

} // namespace matforge::core::signal

