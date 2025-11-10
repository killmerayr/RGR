#pragma once
#include <string>
#include <vector>

// Проверка корректности кодового слова (только ASCII буквы)
bool isValidCodeWord(const std::vector<unsigned char>& codeWord);

// Шифрование текста
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& text, 
                                 const std::vector<unsigned char>& codeWord);

// Дешифрование текста
std::vector<unsigned char> decrypt(const std::vector<unsigned char>& text,
                                 const std::vector<unsigned char>& codeWord);

// Функции пользовательского интерфейса
void process_terminal_codeword(bool encrypt);
void menu_codeword();

// C-style API для динамической загрузки
extern "C" {
    void codewordEncrypt(const std::string& inputPath, const std::string& outputPath);
    void codewordDecrypt(const std::string& inputPath, const std::string& outputPath);
}