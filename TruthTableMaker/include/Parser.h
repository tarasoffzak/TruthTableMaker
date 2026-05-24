/**
 * @file Parser.h
 * @brief Объявление синтаксического анализатора для обратной польской записи (ОПН).
 */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>

// Опережающие объявления для развязки зависимостей
class ExprTree;
class Config;
class FunctManager;
struct UserFunction;

/**
 * @class Parser
 * @brief Синтаксический анализатор выражений в обратной польской записи (ОПН).
 *
 * Отвечает за преобразование строк в формате ОПН в абстрактное синтаксическое дерево,
 * а также за разбор определений пользовательских функций.
 */
class Parser {
private:
    const Config* config;         ///< Указатель на настройки конфигурации.
    const FunctManager* functMgr; ///< Указатель на менеджер пользовательских функций.

public:
    /**
     * @brief Конструктор парсера.
     * @param cfg Указатель на конфигурацию (символы операторов, лимиты).
     * @param fm Указатель на менеджер для поиска уже зарегистрированных функций.
     */
    Parser(const Config* cfg, const FunctManager* fm);

    /**
     * @brief Разбирает логическое выражение в формате ОПН.
     * @param expression Строка с выражением в обратной польской записи (например, "A B & C |").
     * @return Умный указатель на скомпилированное дерево выражения.
     * @throws Error При некорректном синтаксисе ОПН или неизвестных токенах.
     */
    std::shared_ptr<ExprTree> parseExprRPN(const std::string& expression);

    /**
     * @brief Разбирает определение пользовательской функции из строки.
     *
     * Ожидается, что тело функции также описано в формате ОПН.
     * @param definition Строка с определением функции (например, "MYFUNC = x y &").
     * @return Готовая структура UserFunction, которую можно передать в FunctManager.
     * @throws Error При неверном формате определения функции.
     */
    UserFunction parseUserFunction(const std::string& definition);
};

#endif // PARSER_H