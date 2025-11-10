#pragma once
#include <string>
#include <vector>

// Шифрование текста тарабарским языком
std::vector<unsigned char> encrypt_tarabar(const std::vector<unsigned char>& input);

// Дешифрование текста тарабарским языком
std::vector<unsigned char> decrypt_tarabar(const std::vector<unsigned char>& input);

// Пользовательский интерфейс
void process_terminal_tarabar(bool encrypt);
void menu_tarabar();

// C-style API для динамической загрузки
extern "C" {
    void tarabarEncrypt(const std::string& inputPath, const std::string& outputPath);
    void tarabarDecrypt(const std::string& inputPath, const std::string& outputPath);
}