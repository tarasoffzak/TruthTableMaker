/**
 * @file UserFunct.h
 * @brief Объявление структуры UserFunction и класса FunctManager.
 */

#ifndef USER_FUNCT_H
#define USER_FUNCT_H

#include <string>
#include <map>
#include <memory>

// Опережающие объявления
class ExprTree;
struct Config;

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
 * зарегистрированных пользовательских функций, а также их загрузку из файлов.
 */
class FunctManager {
private:
    /**
     * @brief Хранилище зарегистрированных функций.
     *
     * Использование std::map гарантирует стабильность адресов элементов в памяти,
     * что позволяет узлам AST безопасно хранить указатели на UserFunction.
     */
    std::map<std::string, UserFunction> functions;

public:
    /**
     * @brief Конструктор по умолчанию.
     */
    FunctManager() = default;

    /**
     * @brief Деструктор по умолчанию.
     */
    ~FunctManager() = default;

    // Запрещаем копирование для сохранения стабильности указателей на элементы map
    FunctManager(const FunctManager&) = delete;
    FunctManager& operator=(const FunctManager&) = delete;

    /**
     * @brief Загружает пользовательские функции из файла.
     *
     * Читает файл построчно, пропускает пустые строки и комментарии (#).
     * Каждую строку передаёт в parseUserFunction, результат регистрирует.
     * @param filePath Путь к файлу с определениями функций.
     * @param config Конфигурация, передаваемая в парсер ОПН для каждой функции.
     * @throws Error Если файл не найден или содержит синтаксические ошибки.
     */
    void loadFromFile(const std::string& filePath, const Config& config);

    /**
     * @brief Разбирает определение пользовательской функции из строки.
     *
     * Ожидается, что тело функции также описано в формате ОПН.
     * @param definition Строка с определением функции (например, "MYFUNC = x y &").
     * @param config Конфигурация, передаваемая в парсер ОПН.
     * @return Готовая структура UserFunction, которую можно передать в FunctManager.
     * @throws Error При неверном формате определения функции.
     */
    UserFunction parseUserFunction(const std::string& definition, const Config& config);

    /**
     * @brief Регистрирует новую функцию в системе вручную.
     * @param func Структура с данными скомпилированной функции.
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