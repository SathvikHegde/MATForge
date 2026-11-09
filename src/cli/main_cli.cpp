#include "matforge/engine/Interpreter.hpp"
#include "matforge/engine/Lexer.hpp"
#include <iostream>
#include <string>
#include <algorithm>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "========================================================\n";
    std::cout << "  MATForge Interactive CLI v0.1.0\n";
    std::cout << "  MATLAB Clone built with C++20 & MSVC\n";
    std::cout << "  Type 'help' for commands, 'exit' or 'quit' to close.\n";
    std::cout << "========================================================\n\n";

    auto workspace = std::make_shared<matforge::engine::Workspace>();
    matforge::engine::Interpreter interpreter(workspace);

    interpreter.setClearScreenCallback([]() {
#if defined(_WIN32)
        system("cls");
#else
        system("clear");
#endif
    });

    std::string input;
    std::string accumulatedCommand;
    int blockDepth = 0;

    while (true) {
        if (blockDepth == 0) {
            std::cout << ">> ";
        } else {
            std::cout << ".. ";
        }

        if (!std::getline(std::cin, input)) {
            break;
        }

        // Trim whitespace
        auto start = input.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            if (blockDepth == 0) continue;
        }
        std::string trimmed = (start == std::string::npos) ? "" : input.substr(start);

        if (blockDepth == 0 && (trimmed == "exit" || trimmed == "quit")) {
            break;
        }

        accumulatedCommand += input + "\n";

        // Analyze token block depth
        matforge::engine::Lexer lexer(accumulatedCommand);
        auto tokens = lexer.tokenize();
        int openBlocks = 0;
        int closeBlocks = 0;
        for (std::size_t i = 0; i < tokens.size(); ++i) {
            auto t = tokens[i].type;
            if (t == matforge::engine::TokenType::For ||
                t == matforge::engine::TokenType::While ||
                t == matforge::engine::TokenType::If ||
                t == matforge::engine::TokenType::Function) {
                openBlocks++;
            } else if (t == matforge::engine::TokenType::End) {
                bool isIndexEnd = false;
                if (i > 0) {
                    auto prev = tokens[i - 1].type;
                    if (prev == matforge::engine::TokenType::LParen ||
                        prev == matforge::engine::TokenType::Colon ||
                        prev == matforge::engine::TokenType::Comma) {
                        isIndexEnd = true;
                    }
                }
                if (!isIndexEnd) {
                    closeBlocks++;
                }
            }
        }

        blockDepth = std::max(0, openBlocks - closeBlocks);

        if (blockDepth == 0 && !accumulatedCommand.empty()) {
            auto result = interpreter.execute(accumulatedCommand);
            if (!result.success) {
                std::cout << "Error: " << result.errorMessage << "\n\n";
            } else if (!result.output.empty()) {
                std::cout << result.output;
            }
            accumulatedCommand.clear();
        }
    }

    return 0;
}
