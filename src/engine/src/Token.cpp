#include "matforge/engine/Token.hpp"

namespace matforge::engine {

std::string Token::typeName() const {
    switch (type) {
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::Caret: return "^";
        case TokenType::DotStar: return ".*";
        case TokenType::DotSlash: return "./";
        case TokenType::DotCaret: return ".^";
        case TokenType::Transpose: return "'";
        case TokenType::Assign: return "=";
        case TokenType::Equals: return "==";
        case TokenType::NotEquals: return "~=";
        case TokenType::Less: return "<";
        case TokenType::LessEquals: return "<=";
        case TokenType::Greater: return ">";
        case TokenType::GreaterEquals: return ">=";
        case TokenType::LParen: return "(";
        case TokenType::RParen: return ")";
        case TokenType::LBracket: return "[";
        case TokenType::RBracket: return "]";
        case TokenType::Comma: return ",";
        case TokenType::Semicolon: return ";";
        case TokenType::Colon: return ":";
        case TokenType::Newline: return "Newline";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Invalid: return "Invalid";
    }
    return "Unknown";
}

} // namespace matforge::engine

