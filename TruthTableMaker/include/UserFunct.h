/**
 * @file UserFunct.h
 * @brief Объявление структуры UserFunction и класса FunctManager.
 */

#ifndef USER_FUNCT_H
#define USER_FUNCT_H

#include <string>
#include <map>
#include <memory>

// Опережающее объявление: достаточно знать, что класс ExprTree существует.
class ExprTree;

/**
 * @struct UserFunction
 * @brief Структура, описывающая пользовательскую логическую функцию.
 */
struct UserFunction {
    std::string name;                ///< Уникальное имя функции.
    std::shared_ptr<ExprTree> body;  ///< Дерево логического выражения функции.
};

/**
 * @class FunctManager
 * @brief Менеджер пользовательских функций.
 *
 * Отвечает за хранение, поиск и автоматическое управление памятью
 * зарегистрированных пользовательских функций.
 */
class FunctManager {
private:
    /**
     * @brief Хранилище зарегистрированных функций. 
     *
     * Использование std::map гарантирует стабильность адресов элементов в памяти,
     * что позволяет узлам AST (FunctNode) безопасно хранить указатели на UserFunction.
     */
    std::map<std::string, UserFunction> functions;

public:
    /**
     * @brief Конструктор по умолчанию.
     */
    FunctManager() = default;

    /**
     * @brief Деструктор по умолчанию.
     *
     * Память дерева выражений очищается автоматически силами std::shared_ptr.
     */
    ~FunctManager() = default;

    // Запрещаем копирование для сохранения стабильности указателей на элементы map
    FunctManager(const FunctManager&) = delete;
    FunctManager& operator=(const FunctManager&) = delete;

    /**
     * @brief Регистрирует новую функцию в системе.
     * @param func Структура с данными функции.
     * @throws Error Если функция с таким именем уже зарегистрирована.
     */
    void registerFunction(UserFunction func);

    /**
     * @brief Проверяет наличие функции в менеджере.
     * @param name Имя функции для проверки.
     * @return true, если функция найдена, иначе false.
     */
    bool hasFunction(const std::string& name) const;

    /**
     * @brief Возвращает стабильный указатель на зарегистрированную функцию.
     * @param name Имя искомой функции.
     * @return Константный указатель на структуру UserFunction.
     * @throws Error Если функция с таким именем не найдена.
     */
    const UserFunction* getFunction(const std::string& name) const;
};

#endif // USER_FUNCT_H