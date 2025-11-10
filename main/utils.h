#pragma once
#include <string>
#include <vector>

// Проверка символа на ASCII букву
bool isAsciiLetter(unsigned char c);

// Чтение бинарного файла
std::vector<unsigned char> readFileBinary(const std::string& filename);

// Запись бинарного файла
void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer);

// Обратная совместимость для текстовых файлов
std::string readFile(const std::string& filename);
void writeFile(const std::string& filename, const std::string& content);

// Ввод пароля
std::string Password();