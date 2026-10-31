#pragma once

#include "matforge/core/Value.hpp"
#include <string>
#include <unordered_map>
#include <functional>

namespace matforge::engine {

class Workspace {
public:
    using ChangeCallback = std::function<void(const std::string& varName, const matforge::core::Value& value)>;
    using ClearCallback = std::function<void()>;

    Workspace();

    void set(const std::string& name, matforge::core::Value value);
    [[nodiscard]] const matforge::core::Value& get(const std::string& name) const;
    [[nodiscard]] bool has(const std::string& name) const;
    void remove(const std::string& name);
    void clear();

    [[nodiscard]] const std::unordered_map<std::string, matforge::core::Value>& variables() const {
        return variables_;
    }

    void setChangeCallback(ChangeCallback cb) { changeCallback_ = std::move(cb); }
    void setClearCallback(ClearCallback cb) { clearCallback_ = std::move(cb); }

private:
    void initConstants();

    std::unordered_map<std::string, matforge::core::Value> variables_;
    ChangeCallback changeCallback_;
    ClearCallback clearCallback_;
};

} // namespace matforge::engine

