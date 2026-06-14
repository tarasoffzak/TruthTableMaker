/**
 * @file ExprTree.cpp
 * @brief Реализация методов класса ExprTree — управляющего объекта дерева логического выражения.
 */

#include "ExprTree.h"
#include "Error.h"
#include <cstdint>

/// Максимально допустимое количество уникальных переменных для таблицы истинности
/// (предотвращение переполнения памяти).
static const uint32_t maxTruthTableVars = 10;

// ============================================================================
// Реализация ExprTree
// ============================================================================

ExprTree::ExprTree(std::shared_ptr<Node> rootNode, std::vector<std::string> vars)
    : root(std::move(rootNode)), variableNames(std::move(vars)) {}

bool ExprTree::evaluate(Context ctx) const {
    // 1. Делегировать вычисление корневому узлу дерева.
    // 2. Вернуть результат вычисления.
    return root->evaluate(ctx);
}

const std::vector<std::string>& ExprTree::getVariables() const {
    // Вернуть константную ссылку на вектор имён переменных.
    return variableNames;
}

std::vector<std::pair<Context, bool>> ExprTree::generateTruthTable() const {
    // 1. Убедиться, что корень дерева не является нулевым указателем.
    if (!root) {
        ErrorManager::raise(ErrorType::EVAL_ERROR, "Нельзя сгенерировать таблицу, так как дерево пустое");
    }

    // 2. Определить количество переменных.
    const size_t n = variableNames.size();

    // 3. Проверить, что количество переменных не превышает допустимый предел.
    if (n > maxTruthTableVars) {
        ErrorManager::raise(ErrorType::EVAL_ERROR,
            "Превышение лимита количества переменных (" + std::to_string(n) + "/" + std::to_string(maxTruthTableVars) + ")" );
    }

    // 4. Вычислить число строк таблицы как 2^n.
    const Context rowCount = Context(1) << n;

    // 5. Создать результирующий вектор пар (контекст, результат).
    std::vector<std::pair<Context, bool>> table;
    table.reserve(rowCount);

    // 6. Перебрать все возможные комбинации значений переменных.
    for (Context ctx = 0; ctx < rowCount; ++ctx) {
        // 5.1. Вычислить значение выражения для текущего контекста.
        // 5.2. Добавить пару (контекст, результат) в таблицу.
        table.emplace_back(ctx, root->evaluate(ctx));
    }

    // 7. Вернуть заполненную таблицу истинности.
    return table;
}
