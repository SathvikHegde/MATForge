#include "matforge/engine/Interpreter.hpp"
#include <iostream>
#include <string>

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
    while (true) {
        std::cout << ">> ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        // Trim whitespace
        auto start = input.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue;
        }
        auto end = input.find_last_not_of(" \t\r\n");
        std::string command = input.substr(start, end - start + 1);

        if (command == "exit" || command == "quit") {
            break;
        }

        auto result = interpreter.execute(command);
        if (!result.success) {
            std::cout << "Error: " << result.errorMessage << "\n\n";
        } else if (!result.output.empty()) {
            std::cout << result.output;
        }
    }

    return 0;
}

