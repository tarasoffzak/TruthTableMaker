/**
 * @file Error.h
 * @brief Объявление типов ошибок, класса исключения Error и статического менеджера ErrorManager.
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <string>
#include <vector>

/**
 * @enum ErrorType
 * @brief Типы возможных ошибок в программе.
 */
enum class ErrorType {
    SYNTAX_ERROR,   ///< Ошибка синтаксиса
    EVAL_ERROR,     ///< Ошибка вычисления
    CONFIG_ERROR,   ///< Ошибка конфигурации
    FILE_ERROR      ///< Ошибка работы с файлами
};

/**
 * @class Error
 * @brief Класс, представляющий ошибку в программе.
 *
 * Наследуется от std::runtime_error для интеграции со стандартным механизмом 
 * исключений C++. Хранит тип ошибки и подробное описание.
 */
class Error : public std::runtime_error {
private:
    ErrorType type; ///< Категория ошибки.

public:
    /**
     * @brief Конструктор объекта ошибки.
     * @param errType Категория ошибки.
     * @param message Текст сообщения об ошибке.
     */
    Error(ErrorType errType, const std::string& message);

    /**
     * @brief Возвращает тип ошибки.
     * @return Объект перечисления ErrorType.
     */
    ErrorType getType() const;

    /**
     * @brief Возвращает строковое название типа ошибки.
     * @return Строка с текстовым представлением типа (например, "Syntax Error").
     */
    std::string getTypeString() const;
};

/**
 * @class ErrorManager
 * @brief Менеджер для централизованной обработки и накопления ошибок.
 *
 * Предоставляет статические методы для регистрации ошибок, их накопления
 * в памяти и принудительного прерывания работы (через исключения).
 */
class ErrorManager {
private:
    /**
     * @brief Внутреннее хранилище накопленных ошибок. 
     */
    static std::vector<Error> errorHistory;

public:
    // Запрещаем создание объектов этого класса (класс-утилита)
    ErrorManager() = delete;

    /**
     * @brief Регистрирует ошибку и выбрасывает исключение.
     *
     * Добавляет ошибку в историю и немедленно прерывает выполнение.
     * @param type Тип ошибки.
     * @param details Подробное описание.
     * @throws Error Всегда выбрасывает исключение.
     */
    static void raise(ErrorType type, const std::string& details);

    /**
     * @brief Просто добавляет ошибку в историю без прерывания программы.
     * @param type Тип ошибки.
     * @param details Подробное описание.
     */
    static void add(ErrorType type, const std::string& details);

    /**
     * @brief Проверяет, были ли зафиксированы ошибки.
     * @return true, если есть хотя бы одна ошибка, иначе false.
     */
    static bool hasErrors();

    /**
     * @brief Возвращает список всех накопленных ошибок.
     * @return Константная ссылка на вектор с историю ошибок.
     */
    static const std::vector<Error>& getHistory();

    /**
     * @brief Очищает историю ошибок.
     */
    static void clear();

    /**
     * @brief Логирует предупреждение (не сохраняется в истории ошибок).
     * @param details Текст предупреждения.
     */
    static void warning(const std::string& details);
};

#endif // ERROR_H