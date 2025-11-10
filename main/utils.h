#pragma once
#include <string>
#include <vector>

// Проверка символа на букву (ASCII или кириллица)
bool isAsciiLetter(unsigned char c);
bool isLetter(const std::string& text, size_t pos, size_t& charLen);

// Чтение бинарного файла
std::vector<unsigned char> readFileBinary(const std::string& filename);

// Запись бинарного файла
void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer);

// Обратная совместимость для текстовых файлов
std::string readFile(const std::string& filename);
void writeFile(const std::string& filename, const std::string& content);

// Ввод пароля
std::string Password();

// Простое хеширование пароля (для сохранения в файл)
std::string hashPassword(const std::string& password);

// Проверка пароля
bool verifyPassword(const std::string& inputPassword, const std::string& storedHash);