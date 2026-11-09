#pragma once

#include <string>
#include <string_view>

namespace matforge::engine {

enum class TokenType {
    // Literals
    Number,
    String,
    Identifier,

    // Operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Backslash,      // backslash \ (mldivide)
    Caret,          // ^
    DotStar,        // .*
    DotSlash,       // ./
    DotBackslash,   // dot-backslash .\ (ldivide)
    DotCaret,       // .^
    Transpose,      // '
    Assign,         // =
    Equals,         // ==
    NotEquals,      // ~= or !=
    Less,           // <
    LessEquals,     // <=
    Greater,        // >
    GreaterEquals,  // >=

    // Delimiters
    LParen,         // (
    RParen,         // )
    LBracket,       // [
    RBracket,       // ]
    Comma,          // ,
    Semicolon,      // ;
    Colon,          // :
    At,             // @ (function handle)
    Newline,        // \n

    // Keywords
    For,
    While,
    If,
    Else,
    Elseif,
    End,
    Function,
    Return,
    Break,
    Continue,

    // Special
    EndOfFile,
    Invalid
};

struct Token {
    TokenType type{TokenType::Invalid};
    std::string text;
    double numberValue{0.0};
    std::size_t line{1};
    std::size_t column{1};

    [[nodiscard]] std::string typeName() const;
};

} // namespace matforge::engine

