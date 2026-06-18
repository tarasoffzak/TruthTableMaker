/**
 * @file main.cpp
 * @brief Точка входа в программу TruthTableMaker.
 *
 * Поддерживает два режима работы:
 * - **Файловый** (-i <path>): читает RPN-выражение из файла, записывает CSV через -o.
 * - **Интерактивный** (без -i): принимает функции и выражение из stdin,
 *   выводит таблицу в консоль, затем предлагает сохранить результат в файл.
 */

#include "Config.h"
#include "Error.h"
#include "ExprTree.h"
#include "ParseExpr.h"
#include "UserFunct.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <string>
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
static void writeCSV(
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

/**
 * @brief Выводит таблицу истинности в stdout с табуляцией между столбцами.
 *
 * @param vars   Упорядоченный список имён переменных.
 * @param table  Таблица: пары (контекст-битмаска, значение выражения).
 */
static void printTable(
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

/**
 * @brief Сохраняет таблицу истинности в CSV-файл.
 *
 * @param path   Путь к выходному файлу.
 * @param delim  Символ-разделитель столбцов.
 * @param vars   Упорядоченный список имён переменных.
 * @param table  Таблица: пары (контекст-битмаска, значение выражения).
 * @throws Error Если файл не удалось открыть для записи.
 */
static void saveToFile(
    const std::string& path,
    char delim,
    const std::vector<std::string>& vars,
    const std::vector<std::pair<Context, bool>>& table)
{
    std::ofstream out(path);
    if (!out.is_open())
        throw Error(ErrorType::FILE_ERROR, "Не удалось открыть выходной файл: " + path);
    writeCSV(out, delim, vars, table);
    std::cout << "Saved: " << path << '\n';
}

/**
 * @brief Сохраняет лог ошибок в текстовый файл.
 *
 * @param basePath Базовый путь без расширения. Расширение .txt добавляется автоматически.
 */
static void saveErrorLog(const std::string& basePath)
{
    if (basePath.empty()) return;

    std::string errorLogPath = basePath + ".txt";
    std::ofstream errLog(errorLogPath);
    if (errLog.is_open()) {
        const auto& errorHistory = ErrorManager::getHistory();
        for (const auto& err : errorHistory) {
            errLog << "[" << err.getTypeString() << "] " << err.what() << '\n';
        }
        errLog.close();
        std::cout << "Error log saved: " << errorLogPath << '\n';
    }
}

/**
 * @brief Точка входа.
 *
 * Разбирает аргументы командной строки через ConfigManager, затем выбирает режим:
 *
 * **Файловый режим** (если задан -i):
 * 1. Если -f указывает на существующий файл — загружает пользовательские функции.
 * 2. Читает последнюю непустую непрокомментированную строку входного файла как RPN-выражение.
 * 3. Строит AST и генерирует таблицу истинности.
 * 4. Если задан -o — записывает CSV; иначе печатает таблицу в stdout.
 *
 * **Интерактивный режим** (если -i не задан):
 * 1. Считывает определения функций построчно до пустой строки.
 * 2. Считывает RPN-выражение.
 * 3. Строит AST и генерирует таблицу истинности.
 * 4. Печатает таблицу в stdout.
 * 5. Если -o задан — сразу сохраняет в файл; иначе предлагает ввести путь.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив строк-аргументов.
 * @return 0 при успехе, 1 при ошибке.
 */
int main(int argc, char* argv[]) {
    ConfigManager cm;

    try {
        cm.parseCommandLine(argc, argv);
        const Config& cfg = cm.getConfig();

        FunctManager fm;
        std::string expression;

        if (cfg.inputFilePath.empty()) {
            // ===== Интерактивный режим =====
            std::cout << "Functions (one per line, empty line to finish):\n";
            std::string line;
            while (std::getline(std::cin, line)) {
                trim(line);
                if (line.empty()) break;
                if (line[0] == '#') continue;
                fm.registerFunction(fm.parseUserFunction(line, cfg));
            }
            std::cout << "Expression (RPN):\n> ";
            std::getline(std::cin, expression);
            trim(expression);
        } else {
            // ===== Файловый режим =====

            // Загружаем функции только если файл существует — флаг -f необязателен
            {
                std::ifstream funcCheck(cfg.functionsFilePath);
                if (funcCheck.is_open()) {
                    funcCheck.close();
                    fm.loadFromFile(cfg.functionsFilePath, cfg);
                }
            }

            std::ifstream inFile(cfg.inputFilePath);
            if (!inFile.is_open())
                throw Error(ErrorType::FILE_ERROR, "Не удалось открыть входной файл: " + cfg.inputFilePath);

            // Берём последнюю непустую строку, игнорируя комментарии
            std::string line;
            while (std::getline(inFile, line)) {
                trim(line);
                if (!line.empty() && line[0] != '#') expression = line;
            }

            if (expression.empty())
                throw Error(ErrorType::EVAL_ERROR, "Входной файл не содержит выражения: " + cfg.inputFilePath);
        }

        ErrorManager::clear();
        auto tree = parseExprRPN(expression, cfg, fm);
        if (!tree) {
            ErrorManager::printHistory();
            saveErrorLog(cfg.outputFilePath);
            return 1;
        }
        const auto table = tree->generateTruthTable();
        const auto& vars = tree->getVariables();

        if (cfg.inputFilePath.empty()) {
            // Интерактивный: всегда показываем таблицу в консоль
            std::cout << '\n';
            printTable(vars, table);

            if (!cfg.outputFilePath.empty()) {
                // -o задан явно: пишем в файл без вопроса (добавляем .csv)
                std::string csvPath = cfg.outputFilePath + ".csv";
                saveToFile(csvPath, cfg.csvDelimiter, vars, table);
            } else {
                std::cout << "\nSave to file? (path or Enter to skip): ";
                std::string savePath;
                std::getline(std::cin, savePath);
                trim(savePath);
                if (!savePath.empty())
                    saveToFile(savePath, cfg.csvDelimiter, vars, table);
            }
        } else {
            if (cfg.outputFilePath.empty()) {
                // -o не задан: выводим в консоль
                printTable(vars, table);
            } else {
                // -o задан: пишем CSV (добавляем .csv)
                std::string csvPath = cfg.outputFilePath + ".csv";
                std::ofstream out(csvPath);
                if (!out.is_open())
                    throw Error(ErrorType::FILE_ERROR, "Не удалось открыть выходной файл: " + csvPath);
                writeCSV(out, cfg.csvDelimiter, vars, table);
                out.close();
                std::cout << "OK: " << csvPath << '\n';
            }
        }

        return 0;

    } catch (const Error& e) {
        std::cerr << "Error [" << e.getTypeString() << "]: " << e.what() << '\n';
        saveErrorLog(cm.getConfig().outputFilePath);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        saveErrorLog(cm.getConfig().outputFilePath);
        return 1;
    }
}
