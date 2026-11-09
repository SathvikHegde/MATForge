#include "matforge/core/CsvIO.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

namespace matforge::core::io {

Matrix readCSV(const std::string& path) {
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

void writeCSV(const Matrix& mat, const std::string& path, char delimiter) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + path);
    }
    file << std::fixed << std::setprecision(10);
    for (std::size_t r = 0; r < mat.rows(); ++r) {
        for (std::size_t c = 0; c < mat.cols(); ++c) {
            file << mat(r, c);
            if (c + 1 < mat.cols()) {
                file << delimiter;
            }
        }
        file << "\n";
    }
}

} // namespace matforge::core::io

