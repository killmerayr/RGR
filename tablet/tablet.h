#pragma once
#include <string>
#include <vector>
#include <utility>

// Формирование блока
std::vector<unsigned char> MakeBlock(const std::string& content, size_t start, size_t blocksize);

// Перемешивание блока
std::vector<unsigned char> Shuffle(const std::vector<unsigned char>& block, const std::vector<int>& col_key, const std::vector<int>& row_key);


std::pair<std::vector<int>, std::vector<int>> ExpandWithRandom(size_t n, const std::vector<int>& col_key, const std::vector<int>& row_key);

// Шифрование UTF-8 (по символам)
std::vector<uint32_t> Encrypt(const std::vector<uint32_t>& text, size_t block_size, const std::vector<int>& col_key, const std::vector<int>& row_key);

// Обратное перемешивание UTF-8
std::vector<uint32_t> Unshuffle(const std::vector<uint32_t>& block, const std::vector<int>& col_key, const std::vector<int>& row_key);

// Дешифрование UTF-8
std::vector<uint32_t> Decrypt(const std::vector<uint32_t>& encrypted, size_t block_size, const std::vector<int>& col_key, const std::vector<int>& row_key);

// Терминальный ввод
void process_terminal_tablet(bool encrypt);
void menu_tablet();