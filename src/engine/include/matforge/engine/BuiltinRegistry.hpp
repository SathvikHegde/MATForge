#pragma once

#include "matforge/core/Value.hpp"

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace matforge::engine {

class Interpreter;

using BuiltinFunction = std::function<matforge::core::Value(const std::vector<matforge::core::Value>& args, Interpreter& interp)>;

class BuiltinRegistry {
public:
    static BuiltinRegistry& instance();

    [[nodiscard]] bool has(const std::string& name) const;
    matforge::core::Value call(const std::string& name, const std::vector<matforge::core::Value>& args, Interpreter& interp) const;

    void registerFunction(const std::string& name, BuiltinFunction func);

private:
    BuiltinRegistry();
    void registerCoreMath();
    void registerMatrixGenerators();
    void registerLinearAlgebra();
    void registerStatistics();
    void registerSignalProcessing();
    void registerFileIO();
    void registerGraphics();
    void registerSolversAndCalculus();

    std::unordered_map<std::string, BuiltinFunction> functions_;
};

} // namespace matforge::engine

