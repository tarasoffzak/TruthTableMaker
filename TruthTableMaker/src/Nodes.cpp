/**
 * @file Nodes.cpp
 * @brief Реализация методов классов узлов абстрактного синтаксического дерева (AST).
 */

#include "Nodes.h"
#include "UserFunct.h"
#include "ExprTree.h"

// ============================================================================
// Реализация ConstNode
// ============================================================================

ConstNode::ConstNode(bool val) : value(val) {}

bool ConstNode::evaluate(Context /*ctx*/) const {
    // 1. Вернуть сохраненное значение константы.
    return value;
}

// ============================================================================
// Реализация VarNode
// ============================================================================

VarNode::VarNode(VarID idx) : index(idx) {}

bool VarNode::evaluate(Context ctx) const {
    // 1. Сдвинуть единицу влево на величину индекса переменной.
    // 2. Выполнить побитовое логическое умножение («И») текущего контекста и битовой маски.
    // 3. Вернуть результат (приведение к bool).
    return (ctx >> index) & 1;
}

// ============================================================================
// Реализация OperNode
// ============================================================================

OperNode::OperNode(OperatorType operation, std::vector<std::shared_ptr<Node>> args)
    : op(operation), children(std::move(args)) {}

bool OperNode::evaluate(Context ctx) const {
    switch (op) {
        // Если операция — логическое отрицание («НЕ»):
        case OperatorType::NOT:
            if (!children.empty()) {
                // 1.1. Вычислить значение единственного дочернего узла. 
                // 1.2. Вернуть инвертированный результат.
                return !children[0]->evaluate(ctx);
            }
            return false; // Защита от пустого узла

        // Если операция — логическое умножение («И»):
        case OperatorType::AND:
            // 2.1. Для каждого дочернего узла выполнить вычисление.
            for (const std::shared_ptr<Node>& child : children) {
                if (!child->evaluate(ctx)) {
                    return false; 
                }
            }
            // 2.2. Вернуть конъюнкцию результатов дочерних узлов.
            return true;

        // Если операция — логическое сложение («ИЛИ»):
        case OperatorType::OR:
            // 3.1. Для каждого дочернего узла выполнить вычисление.
            for (const std::shared_ptr<Node>& child : children) {
                if (child->evaluate(ctx)) {
                    return true;
                }
            }
            // 3.2. Вернуть дизъюнкцию результатов дочерних узлов.
            return false;

        default:
            return false; // На случай неизвестного оператора
    }
}

// ============================================================================
// Реализация FunctNode
// ============================================================================

FunctNode::FunctNode(const UserFunction* functPtr, std::vector<std::shared_ptr<Node>> args)
    : func(functPtr), children(std::move(args)) {}

bool FunctNode::evaluate(Context ctx) const {
    // 1. Создать новый пустой контекст со значением ноль.
    Context newCtx = 0;
    
    // 2. Установить счетчик аргументов в ноль.
    Arity argCounter = 0;
    
    // 3. Для каждого аргумента выполнить операции:
    for (const auto& arg : children) {
        // 3.1. Вычислить значение аргумента в текущем контексте.
        if (arg->evaluate(ctx)) {
            // 3.2. Если значение равно «истина», установить соответствующий 
            // счетчику бит в новом контексте в единицу.
            newCtx |= (1ULL << argCounter);
        }
        // 3.3. Увеличить счетчик аргументов на единицу.
        argCounter++;
    }
    
    // 4. Передать новый контекст в поддерево тела функции.
    // 5. Вычислить и вернуть результат выполнения поддерева функции.
    return func->body->evaluate(newCtx);
}