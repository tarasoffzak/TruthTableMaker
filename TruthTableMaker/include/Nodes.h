#ifndef AST_NODES_H
#define AST_NODES_H

#include <cstdint>
#include <vector>
#include <string>

class FunctManager;

/**
 * @brief Контекст вычисления выражения.
 * 
 * Представляет собой 64-битное целое число (битовую маску), 
 * где каждый бит (от 0 до 63) соответствует логическому (булеву) значению 
 * переменной с соответствующим индексом.
 */
using Context = uint64_t;

/**
 * @brief Абстрактный базовый класс узла дерева разбора (AST).
 * 
 * От этого класса наследуются все типы узлов логического выражения.
 */
class Node {
public:
    /**
     * @brief Виртуальный деструктор по умолчанию.
     */
    virtual ~Node() = default;
    
    /**
     * @brief Вычисляет логическое значение узла.
     * 
     * @param ctx Контекст (битовая маска значений переменных).
     * @return true если результат вычисления истина, иначе false.
     */
    virtual bool evaluate(Context ctx) const = 0;
};

/**
 * @brief Узел логической константы.
 * 
 * Хранит фиксированное логическое значение (true или false), 
 * которое не зависит от контекста вычисления.
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
 * @brief Узел логической переменной.
 * 
 * Значение переменной извлекается из переданного контекста по ее индексу.
 */
class VarNode : public Node {
private:
    int index; ///< Индекс переменной (номер бита в контексте).

public:
    /**
     * @brief Конструктор узла переменной.
     * @param idx Индекс (порядковый номер) переменной.
     */
    explicit VarNode(int idx);

    /**
     * @brief Извлекает значение переменной из контекста.
     * @param ctx Контекст с текущими значениями всех переменных.
     * @return Значение бита под номером index.
     */
    bool evaluate(Context ctx) const override;
};

/**
 * @brief Поддерживаемые базисные логические операции.
 */
enum class OperatorType {
    AND, ///< Логическое И (конъюнкция)
    OR,  ///< Логическое ИЛИ (дизъюнкция)
    NOT  ///< Логическое НЕ (отрицание)
};

/**
 * @brief Узел базисной логической операции.
 * 
 * Применяет базовый оператор к своим дочерним узлам (операндам).
 */
class OperNode : public Node {
private:
    OperatorType op;             ///< Тип выполняемой операции.
    std::vector<Node*> children; ///< Дочерние узлы (операнды).

public:
    /**
     * @brief Конструктор узла операции.
     * @param operation Тип логической операции (AND, OR, NOT).
     * @param args Вектор указателей на дочерние узлы-операнды. Класс берет ответственность за их удаление.
     */
    OperNode(OperatorType operation, std::vector<Node*> args);

    /**
     * @brief Деструктор. Освобождает память всех дочерних узлов.
     */
    ~OperNode() override;
    
    /**
     * @brief Вычисляет значения дочерних узлов и применяет к ним оператор.
     * @param ctx Текущий контекст вычисления.
     * @return Результат выполнения логической операции.
     */
    bool evaluate(Context ctx) const override;
};

/**
 * @brief Узел вызова пользовательской функции.
 * 
 * Интерпретирует функцию через вычисление ее поддерева в изолированном контексте.
 */
class FunctNode : public Node {
private:
    std::string funcName;        ///< Имя вызываемой функции.
    std::vector<Node*> children; ///< Аргументы функции (поддеревья основного выражения).
    FunctManager& functManager;  ///< Ссылка на менеджер для получения AST функции.

public:
    /**
     * @brief Конструктор узла пользовательской функции.
     * @param name Имя функции.
     * @param args Вектор указателей на узлы-аргументы. Класс берет ответственность за их удаление.
     * @param fm Ссылка на менеджер пользовательских функций.
     */
    FunctNode(std::string name, std::vector<Node*> args, FunctManager& fm);

    /**
     * @brief Деструктор. Освобождает память всех узлов-аргументов.
     */
    ~FunctNode() override;
    
    /**
     * @brief Вычисляет функцию как поддерево.
     * 
     * Оценивает аргументы в текущем основном контексте `ctx`, формирует новый локальный 
     * контекст и вычисляет корень поддерева функции через `FunctManager`.
     * 
     * @param ctx Основной контекст вычисления.
     * @return Результат выполнения пользовательской функции.
     */
    bool evaluate(Context ctx) const override;
};

#endif // AST_NODES_H