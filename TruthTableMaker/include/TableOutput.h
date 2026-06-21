/**
 * @file TableOutput.h
 * @brief Вывод таблицы истинности: печать в консоль и запись в CSV-файл.
 */

#ifndef TABLE_OUTPUT_H
#define TABLE_OUTPUT_H

#include "Types.h"
#include <ostream>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Записывает таблицу истинности в CSV-поток.
 *
 * Первая строка — заголовок: имена переменных + "result".
 * Каждая последующая строка — одна комбинация значений переменных и результат.
 *
 * @param out    Выходной поток (файл или stdout).
 * @param delim  Символ-разделитель столбцов.
 * @param vars   Упорядоченный список имён переменных.
 * @param table  Таблица: пары (контекст-битмаска, значение выражения).
 */
void writeCSV(
    std::ostream& out,
    char delim,
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table);

/**
 * @brief Выводит таблицу истинности в stdout с табуляцией между столбцами.
 *
 * @param vars   Упорядоченный список имён переменных.
 * @param table  Таблица: пары (контекст-битмаска, значение выражения).
 */
void printTable(
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table);

/**
 * @brief Сохраняет таблицу истинности в CSV-файл.
 *
 * @param path   Путь к выходному файлу.
 * @param delim  Символ-разделитель столбцов.
 * @param vars   Упорядоченный список имён переменных.
 * @param table  Таблица: пары (контекст-битмаска, значение выражения).
 * @throws Error Если файл не удалось открыть для записи.
 */
void saveToFile(
    const std::string& path,
    char delim,
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table);

#endif // TABLE_OUTPUT_H
