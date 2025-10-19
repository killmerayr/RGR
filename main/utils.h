#pragma once
#include <string>
#include<vector>
bool isAlpha(uint32_t cp);

// Чтение файла в строку
std::string readFile(const std::string& filename);

// Запись строки в файл
void writeFile(const std::string& filename, const std::string& content);

std::string Password();

void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer);