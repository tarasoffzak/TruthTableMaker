#include "Error.h"
#include <iostream>

// ============================================================================
// Реализация класса Error
// ============================================================================

Error::Error(ErrorType errType, const std::string& message) 
    : std::runtime_error(message), type(errType) {}

ErrorType Error::getType() const {
    return type;
}

std::string Error::getTypeString() const {
    switch (type) {
        case ErrorType::SYNTAX_ERROR: return "Syntax Error";
        case ErrorType::EVAL_ERROR:   return "Evaluation Error";
        case ErrorType::CONFIG_ERROR: return "Configuration Error";
        case ErrorType::FILE_ERROR:   return "File Error";
        default:                      return "Unknown Error";
    }
}

// ============================================================================
// Реализация пространства имён ErrorManager
// ============================================================================

namespace {
    // Внутреннее хранилище накопленных ошибок.
    // Скрыто в анонимном namespace — недоступно за пределами этой единицы трансляции.
    std::vector<Error> errorHistory;
}

namespace ErrorManager {

void raise(ErrorType type, const std::string& details) {
    add(type, details); // Сначала сохраняем в историю
    throw Error(type, details); // Затем прерываем выполнение
}

void add(ErrorType type, const std::string& details) {
    errorHistory.emplace_back(type, details);
}

bool hasErrors() {
    return !errorHistory.empty();
}

const std::vector<Error>& getHistory() {
    return errorHistory;
}

void clear() {
    errorHistory.clear();
}

void warning(const std::string& details) {
    // Предупреждения не ломают программу, просто выводим их в поток ошибок
    std::cerr << "[WARNING] " << details << std::endl;
}

void printHistory() {
    for (const Error& err : errorHistory) {
        std::cerr << "[" << err.getTypeString() << "] " << err.what() << std::endl;
    }
}

} // namespace ErrorManager