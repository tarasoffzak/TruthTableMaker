/**
 * @file Nodes.h
 * @brief Объявление классов узлов абстрактного синтаксического дерева (AST).
 */

#ifndef AST_NODES_H
#define AST_NODES_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "Types.h"

struct UserFunction;

/**
 * @brief Контекст вычисления выражения.
 *
 * Представляет собой 64-битное целое число (битовую маску), 
 * где каждый бит (от 0 до 63) соответствует логическому значению 
 * переменной с соответствующим индексом.
 */
using Context = uint64_t;

/**
 * @class Node
 * @brief Абстрактный базовый класс узла дерева разбора (AST).
 */
class Node {
public:
    /**
     * @brief Виртуальный деструктор по умолчанию.
     */
    virtual ~Node() = default;
    
    /**
     * @brief Вычисляет логическое значение узла.
     * @param ctx Контекст (битовая маска значений переменных).
     * @return true, если результат вычисления истина, иначе false.
     */
    virtual bool evaluate(Context ctx) const = 0;
};

/**
 * @class ConstNode
 * @brief Узел логической константы.
 */
class ConstNode : public Node {
private:
    bool value; ///< Хранимое булево значение.

public:
    /**
     * @brief Конструктор узла константы.
     * @param val Значение константы.
     */
    explicit ConstNode(bool val);

    /**
     * @brief Возвращает значение константы.
     * @param ctx Контекст (не используется).
     * @return Значение константы.
     */
    bool evaluate(Context ctx) const override;
};

/**
 * @class VarNode
 * @brief Узел логической переменной.
 */
class VarNode : public Node {
private:
    size_t index; ///< Индекс переменной (номер бита в контексте от 0 до 63).

public:
    /**
     * @brief Конструктор узла переменной.
     * @param idx Индекс (порядковый номер) переменной.
     */
    explicit VarNode(size_t idx);

    /**
     * @brief Извлекает значение переменной из контекста.
     * @param ctx Контекст с текущими значениями всех переменных.
     * @return Значение бита под номером index.
     */
    bool evaluate(Context ctx) const override;
};

/**
 * @class OperNode
 * @brief Узел базисной логической операции.
 */
class OperNode : public Node {
private:
    OperatorType op;                               ///< Тип выполняемой операции.
    std::vector<std::shared_ptr<Node>> children;   ///< Дочерние узлы (операнды).

public:
    /**
     * @brief Конструктор узла операции.
     * @param operation Тип логической операции (AND, OR, NOT).
     * @param args Вектор умных указателей на дочерние узлы-операнды.
     */
    OperNode(OperatorType operation, std::vector<std::shared_ptr<Node>> args);

    /**
     * @brief Вычисляет значения дочерних узлов и применяет к ним оператор.
     * @param ctx Текущий контекст вычисления.
     * @return Результат выполнения логической операции.
     */
    bool evaluate(Context ctx) const override;
};

/**
 * @class FunctNode
 * @brief Узел вызова пользовательской функции.
 */
class FunctNode : public Node {
private:
    const UserFunction* func;                      ///< Константный указатель на описание функции.
    std::vector<std::shared_ptr<Node>> children;   ///< Аргументы функции (поддеревья).

public:
    /**
     * @brief Конструктор узла пользовательской функции.
     * @param functPtr Указатель на структуру функции из FunctManager.
     * @param args Вектор умных указателей на узлы-аргументы.
     */
    FunctNode(const UserFunction* functPtr, std::vector<std::shared_ptr<Node>> args);
    
    /**
     * @brief Вычисляет значение пользовательской функции в изолированном контексте.
     * @param ctx Текущий контекст вычисления.
     * @return Результат выполнения функции.
     */
    bool evaluate(Context ctx) const override;
};

#endif // AST_NODES_H