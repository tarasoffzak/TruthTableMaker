/**
 * @file UserFunct.cpp
 * @brief Реализация методов класса FunctManager.
 */

#include "UserFunct.h"
#include "ParseExpr.h"
#include "Config.h"
#include "Error.h"
#include "Utils.h"

#include <fstream>

// ============================================================================
// Реализация FunctManager
// ============================================================================

UserFunction FunctManager::parseUserFunction(const std::string& definition, const Config& config) {
    // 1. Найти разделитель '=' между именем функции и телом в ОПН
    const std::size_t delimPos = definition.find('=');
    if (delimPos == std::string::npos) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Неверный формат определения функции (ожидается: <ИМЯ> = <ОПН>): '" + definition + "'");
    }

    // 2. Извлечь и очистить имя и тело функции
    std::string name = definition.substr(0, delimPos);
    std::string body = definition.substr(delimPos + 1);
    trim(name);
    trim(body);

    // 3. Проверить что имя и тело не пустые
    if (name.empty()) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Имя функции не может быть пустым в определении: '" + definition + "'");
    }
    if (body.empty()) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Тело функции не может быть пустым: '" + name + "'");
    }

    // 4. Защита от пробельных символов внутри имени функции
    if (name.find_first_of(" \t\r\n") != std::string::npos) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Имя функции не должно содержать пробелы: '" + name + "'");
    }

    // 5. Проверить корректность имени (запрещённые символы операторов и префикс)
    if (!validateNaming(name, "функции", config)) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Имя функции содержит недопустимые символы: '" + name + "'");
    }

    // 6. Проверить что функция с таким именем ещё не зарегистрирована
    if (hasFunction(name)) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Функция '" + name + "' уже зарегистрирована");
    }

    // 7. Построить дерево выражения из тела функции в формате ОПН.
    //    Используем переданную конфигурацию и текущий менеджер функций
    //    (чтобы тело могло ссылаться на ранее загруженные функции).
    std::shared_ptr<ExprTree> tree = parseExprRPN(body, config, *this);

    // 8. Если парсер накопил ошибки — дерево не построено
    if (!tree) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Не удалось разобрать тело функции '" + name + "'");
    }

    return UserFunction{name, std::move(tree)};
}

void FunctManager::registerFunction(UserFunction func) {
    const std::string key = func.name;
    functions.emplace(key, std::move(func));
}

bool FunctManager::hasFunction(const std::string& name) const {
    return functions.count(name) > 0;
}

const UserFunction* FunctManager::getFunction(const std::string& name) const {
    const auto it = functions.find(name);
    if (it == functions.end()) {
        throw Error(ErrorType::SYNTAX_ERROR,
            "Функция '" + name + "' не найдена");
    }
    return &it->second;
}

void FunctManager::loadFromFile(const std::string& filePath, const Config& config) {
    // 1. Открыть файл с определениями функций (критическая ошибка, если не удалось)
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw Error(ErrorType::FILE_ERROR,
            "Не удалось открыть файл функций: " + filePath);
    }

    std::string line;
    uint32_t lineNumber = 0;

    // 2. Читать построчно
    while (std::getline(file, line)) {
        lineNumber++;
        trim(line);

        // Пропускаем пустые строки и комментарии (начинающиеся с '#')
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 3. Разобрать определение и зарегистрировать функцию.
        //    Ошибки отдельных строк накапливаем и продолжаем обработку файла.
        try {
            UserFunction func = parseUserFunction(line, config);
            registerFunction(std::move(func));
        }
        catch (const Error& e) {
            ErrorManager::add(e.getType(),
                "Строка " + std::to_string(lineNumber) + ": " + e.what());
        }
    }
}
