/**
 * @file Types.h
 * @brief Объявление общих перечислений и типов данных проекта.
 */

#ifndef TYPES_H
#define TYPES_H

/**
 * @enum OperatorType
 * @brief Поддерживаемые базисные логические операции.
 */
enum class OperatorType {
    AND, ///< Логическое И (конъюнкция).
    OR,  ///< Логическое ИЛИ (дизъюнкция).
    NOT  ///< Логическое НЕ (отрицание).
};

#endif // TYPES_H