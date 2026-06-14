/**
 * @file Config.cpp
 * @brief Реализация методов класса ConfigManager.
 */

#include "Config.h"
#include "Error.h"
#include "Utils.h"
#include <fstream>

// ============================================================================
// Реализация Config
// ============================================================================

bool Config::isOperator(const std::string& token) const {
    return operators.count(token) > 0;
}

Arity Config::getOperatorArity(const std::string& token) const {
    if (!isOperator(token)) {
        throw Error(ErrorType::SYNTAX_ERROR, "Неизвестный оператор: '" + token + "'");
    }
    return operators.at(token).arity;
}

// ============================================================================
// Реализация ConfigManager
// ============================================================================

ConfigManager::ConfigManager() {
    // Внутренний объект currentConfig автоматически инициализируется
    // значениями по умолчанию, заданными прямо в структуре Config (в Config.h).
}

const Config& ConfigManager::getConfig() const {
    return currentConfig;
}

void ConfigManager::parseCommandLine(int argc, char* argv[]) {
    // Проходим по всем аргументам командной строки (начиная с индекса 1, т.к. индекс 0 — имя программы)
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        // Проверяем флаг для пути к выходному файлу
        if (arg == "--output" || arg == "-o") {
            // Переходим к следующему аргументу (значению флага)
            if (++i >= argc) {
                // Если значения нет, выбрасываем ошибку
                throw Error(ErrorType::SYNTAX_ERROR, "Флаг '" + arg + "' требует значения");
            }
            // Сохраняем путь к выходному файлу
            currentConfig.outputFilePath = argv[i];
        }
        // Проверяем флаг для пути к файлу пользовательских функций
        else if (arg == "--functions" || arg == "-f") {
            // Переходим к следующему аргументу (значению флага)
            if (++i >= argc) {
                // Если значения нет, выбрасываем ошибку
                throw Error(ErrorType::SYNTAX_ERROR, "Флаг '" + arg + "' требует значения");
            }
            // Сохраняем путь к файлу функций
            currentConfig.functionsFilePath = argv[i];
        }
        // Проверяем флаг для пути к входному файлу с выражением
        else if (arg == "--input" || arg == "-i") {
            if (++i >= argc) {
                throw Error(ErrorType::SYNTAX_ERROR, "Флаг '" + arg + "' требует значения");
            }
            currentConfig.inputFilePath = argv[i];
        }
        // Если аргумент не распознан
        else {
            throw Error(ErrorType::SYNTAX_ERROR, "Неизвестный аргумент: '" + arg + "'");
        }
    }
}


void ConfigManager::loadFromFile(const std::string& filePath) {
    // Открываем файл конфигурации для чтения
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw Error(ErrorType::FILE_ERROR, "Не удалось открыть файл конфигурации: " + filePath);
    }

    std::string line;
    uint32_t lineNumber = 0;

    // Читаем конфигурацию построчно
    while (std::getline(file, line)) {
        lineNumber++;
        trim(line);

        // Пропускаем пустые строки и комментарии (начинающиеся с #)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Ищем разделитель '=' между ключом и значением
        std::size_t delimPos = line.find('=');
        if (delimPos == std::string::npos) {
            throw Error(ErrorType::SYNTAX_ERROR, "Неверный формат строки " + std::to_string(lineNumber) + " (ожидается key=value)");
        }

        // Извлекаем ключ и значение, удаляем пробелы
        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 1);
        trim(key);
        trim(value);

        // Обрабатываем известные ключи конфигурации
        if (key == "output_file") {
            currentConfig.outputFilePath = value;
        }
        else if (key == "functions_file") {
            currentConfig.functionsFilePath = value;
        }
        else if (key == "csv_delimiter") {
            // Разделитель должен быть ровно одним символом
            if (value.empty()) {
                throw Error(ErrorType::SYNTAX_ERROR, "csv_delimiter не может быть пустым (строка " + std::to_string(lineNumber) + ")");
            }
            currentConfig.csvDelimiter = value[0];
        }
        else {
            // Неизвестный ключ — выбрасываем ошибку
            throw Error(ErrorType::SYNTAX_ERROR, "Неизвестный ключ конфигурации '" + key + "' в строке " + std::to_string(lineNumber));
        }
    }
}