/**
 * @file Utils.cpp
 * @brief Реализация вспомогательных утилит.
 */

#include "Utils.h"

void trim(std::string& s) {
    // UTF-8 BOM (EF BB BF) - добавляется PowerShell 5.1 и Блокнотом в начало файла
    if (s.starts_with("\xEF\xBB\xBF"))
        s.erase(0, 3);

    const char* whitespace = " \t\r\n";
    s.erase(0, s.find_first_not_of(whitespace));
    s.erase(s.find_last_not_of(whitespace) + 1);
}
