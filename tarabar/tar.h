#pragma once
#include <string>

// Шифрование текста тарабарским языком
std::string encrypt_tarabar(const std::string& input);

// Дешифрование текста тарабарским языком
std::string decrypt_tarabar(const std::string& input);

// Обработка файла (шифрование/дешифрование)
bool process_file_tarabar(const std::string& input_filename, const std::string& output_filename, bool encrypt);

// Обработка ввода-вывода в терминале (шифрование/дешифрование)
void process_terminal_tarabar(bool encrypt);

void menu_tarabar();

