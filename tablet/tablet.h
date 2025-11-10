#pragma once
#include <string>
#include <vector>

// Константы для размера блока
static constexpr size_t TABLET_BLOCK_SIZE = 64;  // 8x8
static constexpr size_t TABLET_BLOCK_DIM = 8;    // Размерность матрицы

// Шифрование табличной перестановкой
std::vector<unsigned char> Encrypt(const std::vector<unsigned char>& text, 
                                  size_t block_size, 
                                  const std::vector<int>& col_key, 
                                  const std::vector<int>& row_key);

// Дешифрование табличной перестановкой
std::vector<unsigned char> Decrypt(const std::vector<unsigned char>& encrypted, 
                                  size_t block_size, 
                                  const std::vector<int>& col_key, 
                                  const std::vector<int>& row_key);

// Генерирование случайных ключей
std::vector<int> generateRandomKey(size_t size);

// Пользовательский интерфейс
void process_terminal_tablet(bool encrypt);
void menu_tablet();

// C-style API для динамической загрузки
extern "C" {
    void tabletEncrypt(const std::string& inputPath, const std::string& outputPath);
    void tabletDecrypt(const std::string& inputPath, const std::string& outputPath);
}