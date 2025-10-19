#pragma once
#include <string>
#include <vector>

// Проверка уникальности букв в кодовом слове
bool isValidCodeWord(const std::string& codeWord);

// Шифрование текста
std::string encrypt(const std::string& text, const std::string& codeWord);

// Дешифрование текста
std::string decrypt(const std::string& text, const std::string& codeWord);


void process_terminal_codeword(bool encrypt);

void menu_codeword();

std::vector<uint32_t> stringToCodepoints(const std::string& str);