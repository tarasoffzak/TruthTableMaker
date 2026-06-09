/**
 * @file ParseExpr.cpp
 * @brief Реализация функции построения дерева выражения из строки ОПН.
 */

#include "ParseExpr.h"
#include "ExprTree.h"
#include "Nodes.h"
#include "Config.h"
#include "Error.h"
#include "UserFunct.h"
#include "Types.h"

#include <sstream>
#include <stack>
#include <vector>
#include <string>
#include <map>
#include <memory>

/**
 * @brief Вспомогательная функция для валидации имён переменных и функций.
 * @param name Проверяемое имя.
 * @param context Контекст для вывода понятной ошибки ("переменной" или "функции").
 * @param config Конфигурация с правилами парсинга.
 * @return true если имя корректно, false если обнаружены запрещённые символы.
 */
bool validateNaming(const std::string& name, const std::string& context, const Config& config) {
    bool valid = true;

    // Проверка на наличие префикса функции внутри имени
    if (name.find(config.userFuncPrefix) != std::string::npos) {
        ErrorManager::add(ErrorType::SYNTAX_ERROR,
            "Имя " + context + " '" + name + "' не может содержать символ префикса '"
            + config.userFuncPrefix + "'");
        valid = false;
    }

    // Проверка на наличие символов операторов внутри имени
    for (const auto& [opStr, opInfo] : config.operators) {
        if (name.find(opStr) != std::string::npos) {
            ErrorManager::add(ErrorType::SYNTAX_ERROR,
                "Имя " + context + " '" + name + "' не может содержать оператор '" + opStr + "'");
            valid = false;
        }
    }

    return valid;
}

std::shared_ptr<ExprTree> parseExprRPN(
    const std::string& expression,
    const Config& config,
    const FunctManager& functManager)
{
    // 1. Создать пустой стек для узлов дерева.
    std::stack<std::shared_ptr<Node>> nodeStack;
    std::vector<std::string> varNames;
    std::map<std::string, VarID> varIndex;
    VarID nextVarID = 0;

    // 2. Разбить строку выражения на отдельные токены по пробелам.
    std::istringstream stream(expression);
    std::string token;

    // 3. Выполнить цикл для каждого токена.
    while (stream >> token) {

        // 3.1. Если токен является константой ("1" или "0"):
        if (token == "1" || token == "0") {
            // 3.1.1. Создать узел константы.
            // 3.1.2. Поместить узел в стек.
            nodeStack.push(std::make_shared<ConstNode>(token == "1"));
        }
        // 3.2. Если токен является оператором:
        else if (config.isOperator(token)) {
            // 3.2.1. Сравнить количество элементов в стеке с требуемой арностью оператора.
            Arity arity = config.getOperatorArity(token);
            // 3.2.2. Если элементов недостаточно, вызвать ошибку и завершить алгоритм.
            if (nodeStack.size() < arity) {
                ErrorManager::add(ErrorType::SYNTAX_ERROR,
                    "Недостаточно операндов для оператора '" + token + "': "
                    "ожидается " + std::to_string(arity) +
                    ", доступно " + std::to_string(nodeStack.size()));
                continue;
            }
            // 3.2.3. Извлечь из стека необходимое количество узлов-аргументов.
            std::vector<std::shared_ptr<Node>> args(arity);
            for (size_t i = arity; i > 0; --i) {
                args[i - 1] = nodeStack.top();
                nodeStack.pop();
            }
            // 3.2.4. Создать операторный узел, передав ему извлеченные аргументы.
            OperatorType opType = config.operators.at(token).type;
            // 3.2.5. Поместить созданный узел в стек.
            nodeStack.push(std::make_shared<OperNode>(opType, std::move(args)));
        }
        // 3.3. Если токен содержит префикс функции:
        else if (token[0] == config.userFuncPrefix) {
            // 3.3.1. Удалить префикс из токена, чтобы выделить имя функции.
            std::string funcName = token.substr(1);

            // Префикс не может стоять отдельно
            if (funcName.empty()) {
                ErrorManager::add(ErrorType::SYNTAX_ERROR,
                    "Символ '" + std::string(1, config.userFuncPrefix) + "' не может стоять отдельно");
                continue;
            }

            // Проверяем синтаксическую корректность имени функции
            validateNaming(funcName, "функции", config);

            // 3.3.2. Проверить наличие этой функции в менеджере функций.
            // 3.3.3. Если функция отсутствует, вызвать ошибку и завершить алгоритм.
            if (!functManager.hasFunction(funcName)) {
                ErrorManager::add(ErrorType::SYNTAX_ERROR,
                    "Неизвестная функция: '" + funcName + "'");
                continue;
            }
            const UserFunction* func = functManager.getFunction(funcName);
            Arity arity = func->body->getVariables().size();
            if (nodeStack.size() < arity) {
                ErrorManager::add(ErrorType::SYNTAX_ERROR,
                    "Недостаточно аргументов для функции '" + funcName + "': "
                    "ожидается " + std::to_string(arity) +
                    ", доступно " + std::to_string(nodeStack.size()));
                continue;
            }
            // 3.3.4. Извлечь из стека узлы-аргументы согласно арности функции.
            std::vector<std::shared_ptr<Node>> args(arity);
            for (size_t i = arity; i > 0; --i) {
                args[i - 1] = nodeStack.top();
                nodeStack.pop();
            }
            // 3.3.5. Создать функциональный узел с данными аргументами.
            // 3.3.6. Поместить узел в стек.
            nodeStack.push(std::make_shared<FunctNode>(func, std::move(args)));
        }
        // 3.4. Если токен является переменной:
        else {
            // Проверяем синтаксическую корректность имени переменной
            validateNaming(token, "переменной", config);

            // 3.4.1. Получить или создать уникальный числовой индекс для имени переменной.
            auto it = varIndex.find(token);
            VarID id;
            if (it == varIndex.end()) {
                id = nextVarID++;
                varIndex[token] = id;
                varNames.push_back(token);
            } else {
                id = it->second;
            }
            // 3.4.2. Создать узел переменной, используя этот индекс.
            // 3.4.3. Поместить узел в стек.
            nodeStack.push(std::make_shared<VarNode>(id));
        }
    }

    // 4. Проверить количество элементов, оставшихся в стеке.
    // 5. Если в стеке находится не один элемент, вызвать ошибку и завершить алгоритм.
    if (nodeStack.size() != 1) {
        ErrorManager::add(ErrorType::SYNTAX_ERROR,
            "Некорректное выражение ОПН: после разбора в стеке осталось " +
            std::to_string(nodeStack.size()) + " элемент(ов), ожидался 1");
    }

    // Если есть ошибки не строим дерево
    if (ErrorManager::hasErrors()) {
        return nullptr;
    }

    // 6. Извлечь единственный оставшийся узел из стека. (с защитой от пустого стека)
    std::shared_ptr<Node> root = nodeStack.empty() ? nullptr : nodeStack.top();
    // 7. Сформировать и вернуть объект дерева выражения, назначив извлеченный узел корнем.
    return std::make_shared<ExprTree>(std::move(root), std::move(varNames));
}
