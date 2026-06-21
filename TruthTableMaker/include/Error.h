/**
 * @file Error.h
 * @brief Объявление типов ошибок, класса исключения Error и пространства имён ErrorManager.
 */

#ifndef ERROR_H
#define ERROR_H

#include <iostream>
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
 * @namespace ErrorManager
 * @brief Пространство имён для централизованной обработки и накопления ошибок.
 *
 * Предоставляет свободные функции для регистрации ошибок, их накопления
 * в памяти и принудительного прерывания работы (через исключения).
 * Внутреннее хранилище истории скрыто в анонимном namespace (Error.cpp).
 */
namespace ErrorManager {
    /**
     * @brief Регистрирует ошибку и выбрасывает исключение.
     *
     * Добавляет ошибку в историю и немедленно прерывает выполнение.
     * @param type Тип ошибки.
     * @param details Подробное описание.
     * @throws Error Всегда выбрасывает исключение.
     */
    void raise(ErrorType type, const std::string& details);

    /**
     * @brief Просто добавляет ошибку в историю без прерывания программы.
     * @param type Тип ошибки.
     * @param details Подробное описание.
     */
    void add(ErrorType type, const std::string& details);

    /**
     * @brief Проверяет, были ли зафиксированы ошибки.
     * @return true, если есть хотя бы одна ошибка, иначе false.
     */
    bool hasErrors();

    /**
     * @brief Возвращает список всех накопленных ошибок.
     * @return Константная ссылка на вектор с историю ошибок.
     */
    const std::vector<Error>& getHistory();

    /**
     * @brief Очищает историю ошибок.
     */
    void clear();

    /**
     * @brief Логирует предупреждение (не сохраняется в истории ошибок).
     * @param details Текст предупреждения.
     */
    void warning(const std::string& details);

    /**
     * @brief Выводит все накопленные ошибки из истории в заданный поток.
     * @param os Поток для вывода. По умолчанию std::cerr.
     */
    void printHistory(std::ostream& os = std::cerr);

    /**
     * @brief Сохраняет историю ошибок в текстовый файл.
     *
     * Если basePath пуст — ничего не делает. К пути добавляется расширение .txt.
     * @param basePath Базовый путь без расширения.
     */
    void saveToFile(const std::string& basePath);
}

#endif // ERROR_H