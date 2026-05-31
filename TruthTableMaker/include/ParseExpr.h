/**
 * @file ParseExpr.h
 * @brief Объявление функций синтаксического анализа логических выражений.
 */

#ifndef PARSEEXPR_H
#define PARSEEXPR_H

#include <string>
#include <memory>

// Опережающие объявления для развязки зависимостей
class ExprTree;
struct Config;

/**
 * @brief Строит дерево логического выражения из строки в формате ОПН.
 *
 * @param expression Строка в формате ОПН с пробелами-разделителями.
 * @param config Объект конфигурации с набором операторов и настроек парсинга.
 * @return std::shared_ptr<ExprTree> Указатель на корень построенного дерева.
 * @throws Error Если выражение некорректно, содержит неизвестные элементы 
 * или стек вычислений остался в невалидном состоянии.
 */
std::shared_ptr<ExprTree> parseExprRPN(const std::string& expression, const Config& config);

#endif // PARSEEXPR_H