#pragma once
#include <string>
#include <vector>

// Проверка корректности кодового слова (буквы ASCII или кириллица, все уникальные)
// Алгоритм: подстановочный шифр с перестановкой алфавита
// Кодовое слово вставляется в начало алфавита, остальные буквы сдвигаются
// Пример для англ: кодовое слово "мир" -> алфавит: "мира бвгд...xyz"
// Поддерживает смешанный текст (англ + русский)
bool isValidCodeWord(const std::vector<unsigned char>& codeWord);

// Шифрование текста с подстановкой букв согласно таблице алфавита
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& text, 
                                 const std::vector<unsigned char>& codeWord);

// Дешифрование текста с обратной подстановкой
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