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
        ErrorManager::raise(ErrorType::SYNTAX_ERROR, "Неизвестный оператор: '" + token + "'");
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

void ConfigManager::applySetting(const std::string& key, const std::string& value) {
    // Обрабатываем известные ключи конфигурации
    if (key == "output_file") {
        currentConfig.outputFilePath = value;
    }
    else if (key == "functions_file") {
        currentConfig.functionsFilePath = value;
    }
    else if (key == "input_file") {
        currentConfig.inputFilePath = value;
    }
    else if (key == "csv_delimiter") {
        // Разделитель должен быть ровно одним символом
        if (value.empty()) {
            ErrorManager::raise(ErrorType::CONFIG_ERROR, "csv_delimiter не может быть пустым");
        }
        currentConfig.csvDelimiter = value[0];
    }
    else {
        // Неизвестный ключ — выбрасываем ошибку
        ErrorManager::raise(ErrorType::CONFIG_ERROR, "Неизвестный ключ конфигурации '" + key + "'");
    }
}

void ConfigManager::parseCommandLine(int argc, char* argv[]) {
    // Параметры командной строки имеют приоритет над файлом конфигурации
    // независимо от порядка флагов. Поэтому сначала собираем все настройки
    // из аргументов, и только потом загружаем конфиг, а CLI-значения
    // применяем поверх него, чтобы они всегда потому что ни приоритет.
    std::string configPath;                                // путь из флага -c (пустой — флаг не задан)
    std::vector<std::pair<std::string, std::string>> cliSettings; // настройки CLI в виде "ключ-значение"

    // Проходим по всем аргументам (с индекса 1, т.к. индекс 0 — имя программы).
    // Все поддерживаемые флаги требуют значение, поэтому сразу берём следующий
    // аргумент как значение текущего флага.
    for (int i = 1; i < argc; ++i) {
        const std::string flag = argv[i];

        if (++i >= argc) {
            ErrorManager::raise(ErrorType::CONFIG_ERROR, "Флаг '" + flag + "' требует значения");
        }
        const std::string value = argv[i];

        if (flag == "--config" || flag == "-c") {
            configPath = value;
        }
        else if (flag == "--output" || flag == "-o") {
            cliSettings.emplace_back("output_file", value);
        }
        else if (flag == "--functions" || flag == "-f") {
            cliSettings.emplace_back("functions_file", value);
        }
        else if (flag == "--input" || flag == "-i") {
            cliSettings.emplace_back("input_file", value);
        }
        else {
            ErrorManager::raise(ErrorType::CONFIG_ERROR, "Неизвестный аргумент: '" + flag + "'");
        }
    }

    // Сначала применяем файл конфигурации (если задан),
    if (!configPath.empty()) {
        loadFromFile(configPath);
    }
    // затем — параметры командной строки, перекрывая значения из файла.
    for (const auto& [key, value] : cliSettings) {
        applySetting(key, value);
    }
}


void ConfigManager::parseFile(std::istream& in) {
    std::string line;
    uint32_t lineNumber = 0;

    // Читаем конфигурацию построчно
    while (std::getline(in, line)) {
        lineNumber++;
        trim(line);

        // Пропускаем пустые строки и комментарии (начинающиеся с #)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Ищем разделитель '=' между ключом и значением
        std::size_t delimPos = line.find('=');
        if (delimPos == std::string::npos) {
            ErrorManager::raise(ErrorType::CONFIG_ERROR, "Неверный формат строки " + std::to_string(lineNumber) + " (ожидается key=value)");
        }

        // Извлекаем ключ и значение, удаляем пробелы
        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 1);
        trim(key);
        trim(value);

        // Пустой ключ (строка вида "=value") — некорректен
        if (key.empty()) {
            ErrorManager::raise(ErrorType::CONFIG_ERROR, "Пустой ключ в строке " + std::to_string(lineNumber));
        }

        // Интерпретируем пару «ключ-значение» через общий обработчик
        applySetting(key, value);
    }
}

void ConfigManager::loadFromFile(const std::string& filePath) {
    // Открываем файл конфигурации для чтения
    std::ifstream file(filePath);
    if (!file.is_open()) {
        ErrorManager::raise(ErrorType::FILE_ERROR, "Не удалось открыть файл конфигурации: " + filePath);
    }

    // Разбор содержимого делегируем parseFile
    parseFile(file);
}