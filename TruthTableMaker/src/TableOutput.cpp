/**
 * @file TableOutput.cpp
 * @brief Реализация вывода таблицы истинности.
 */

#include "TableOutput.h"
#include "Error.h"
#include <fstream>
#include <iostream>

void writeCSV(
    std::ostream& out,
    char delim,
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table)
{
    const size_t n = vars.size();
    for (size_t i = 0; i < n; ++i) out << vars[i] << delim;
    out << "result\n";
    for (const auto& [ctx, val] : table) {
        // Бит i контекста = значение переменной с индексом i
        for (size_t i = 0; i < n; ++i) out << ((ctx >> i) & 1) << delim;
        out << (val ? 1 : 0) << '\n';
    }
}

void printTable(
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table)
{
    const size_t n = vars.size();
    for (size_t i = 0; i < n; ++i) std::cout << vars[i] << '\t';
    std::cout << "result\n";
    for (const auto& [ctx, val] : table) {
        for (size_t i = 0; i < n; ++i) std::cout << ((ctx >> i) & 1) << '\t';
        std::cout << (val ? 1 : 0) << '\n';
    }
}

void saveToFile(
    const std::string& path,
    char delim,
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table)
{
    std::ofstream out(path);
    if (!out.is_open())
        ErrorManager::raise(ErrorType::FILE_ERROR, "Не удалось открыть выходной файл: " + path);
    writeCSV(out, delim, vars, table);
    std::cout << "Saved: " << path << '\n';
}
