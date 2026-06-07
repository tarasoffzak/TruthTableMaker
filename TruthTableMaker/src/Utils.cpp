/**
 * @file Utils.cpp
 * @brief Реализация вспомогательных утилит.
 */

#include "Utils.h"

void trim(std::string& s) {
    // Удаляем пробелы с начала строки
    const char* whitespace = " \t\r\n";
    s.erase(0, s.find_first_not_of(whitespace));

    // Удаляем пробелы с конца строки
    s.erase(s.find_last_not_of(whitespace) + 1);
}
