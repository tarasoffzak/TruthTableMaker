/**
 * @file ExprTree.h
 * @brief Интерфейс класса ExprTree.
 */

#ifndef EXPR_TREE_H
#define EXPR_TREE_H

#include <vector>
#include <string>
#include <memory>
#include "Nodes.h"

/**
 * @class ExprTree
 * @brief Управляющий объект для дерева логического выражения.
 */
class ExprTree {
private:
    std::shared_ptr<Node> root;
    std::vector<std::string> variableNames;

public:
    /**
     * @brief Конструктор дерева выражений.
     * @param rootNode Корень дерева (AST).
     * @param vars Список имен переменных (индекс == номер бита).
     */
    ExprTree(std::shared_ptr<Node> rootNode, std::vector<std::string> vars);

    /**
     * @brief Вычисляет значение выражения.
     * @param ctx Битовая маска значений переменных.
     * @return true, если результат вычисления истина, иначе false.
     */
    bool evaluate(Context ctx) const;

    /**
     * @brief Возвращает список имен переменных.
     * @return Константная ссылка на вектор имен переменных.
     */
    const std::vector<std::string>& getVariables() const;

    /**
     * @brief Генерирует таблицу истинности для выражения.
     * @return Вектор пар: (битовая маска контекста, результат вычисления).
     */
    std::vector<std::pair<Context, bool>> generateTruthTable() const;
};

#endif // EXPR_TREE_H