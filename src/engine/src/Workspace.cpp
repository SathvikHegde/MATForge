#include "matforge/engine/Workspace.hpp"
#include <stdexcept>
#include <cmath>
#include <limits>

namespace matforge::engine {

Workspace::Workspace() {
    initConstants();
}

void Workspace::initConstants() {
    variables_["pi"] = matforge::core::Value(3.14159265358979323846);
    variables_["e"] = matforge::core::Value(2.71828182845904523536);
    variables_["eps"] = matforge::core::Value(std::numeric_limits<double>::epsilon());
    variables_["Inf"] = matforge::core::Value(std::numeric_limits<double>::infinity());
    variables_["NaN"] = matforge::core::Value(std::numeric_limits<double>::quiet_NaN());
    variables_["ans"] = matforge::core::Value();
}

void Workspace::set(const std::string& name, matforge::core::Value value) {
    variables_[name] = value;
    if (changeCallback_) {
        changeCallback_(name, value);
    }
}

const matforge::core::Value& Workspace::get(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    throw std::runtime_error("Undefined function or variable '" + name + "'.");
}

bool Workspace::has(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

void Workspace::remove(const std::string& name) {
    variables_.erase(name);
}

void Workspace::clear() {
    variables_.clear();
    initConstants();
    if (clearCallback_) {
        clearCallback_();
    }
}

} // namespace matforge::engine

