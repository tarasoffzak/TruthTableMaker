#ifndef TYPES_H
#define TYPES_H

/**
 * @brief Поддерживаемые базисные логические операции.
 */
enum class OperatorType {
    AND, ///< Логическое И (конъюнкция)
    OR,  ///< Логическое ИЛИ (дизъюнкция)
    NOT  ///< Логическое НЕ (отрицание)
};

#endif // TYPES_H