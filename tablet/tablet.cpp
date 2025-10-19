#include "tablet.h"
#include "../main/utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>

#include "utf8cpp/utf8.h"

using namespace std;

// Запись бинарного файла UTF-8 из vector<uint32_t>
void writeFileBinary(const string& filename, const vector<uint32_t>& buffer) {
    ofstream out(filename, ios::binary);
    if (!out.is_open()) throw runtime_error("Не удалось открыть файл для записи: " + filename);
    string encoded;
    utf8::utf32to8(buffer.begin(), buffer.end(), back_inserter(encoded));
    out.write(encoded.data(), encoded.size());
    out.close();
}

// Создание блока
vector<uint32_t> MakeBlock(const vector<uint32_t>& content, size_t start, size_t blocksize){
    vector<uint32_t> block(blocksize, 0);
    for (size_t i = 0; i < blocksize && start + i < content.size(); i++){
        block[i] = content[start + i];
    }
    return block;
}

// Перемешивание блока
vector<uint32_t> Shuffle(const vector<uint32_t>& block, const vector<int>& col_key, const vector<int>& row_key) {
    size_t n = static_cast<size_t>(sqrt(block.size()));
    vector<uint32_t> temp = block;
    vector<uint32_t> new_block(block.size());

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i * n + j < block.size() && i * n + col_key[j] < block.size())
                temp[i * n + col_key[j]] = block[i * n + j];
        }
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (row_key[i] * n + j < block.size() && i * n + j < block.size())
                new_block[row_key[i] * n + j] = temp[i * n + j];
        }
    }

    return new_block;
}

// Обратное перемешивание
vector<uint32_t> Unshuffle(const vector<uint32_t>& block, const vector<int>& col_key, const vector<int>& row_key) {
    size_t n = static_cast<size_t>(sqrt(block.size()));
    vector<uint32_t> temp(block.size());
    vector<uint32_t> new_block(block.size());

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (row_key[i] * n + j < block.size() && i * n + j < block.size())
                temp[i * n + j] = block[row_key[i] * n + j];
        }
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i * n + col_key[j] < block.size() && i * n + j < block.size())
                new_block[i * n + j] = temp[i * n + col_key[j]];
        }
    }

    return new_block;
}

// Шифрование
vector<uint32_t> Encrypt(const vector<uint32_t>& text, size_t block_size, const vector<int>& col_key, const vector<int>& row_key){
    vector<uint32_t> result;
    for(size_t start = 0; start < text.size(); start += block_size){
        vector<uint32_t> block = MakeBlock(text, start, block_size);
        vector<uint32_t> shuffled = Shuffle(block, col_key, row_key);
        result.insert(result.end(), shuffled.begin(), shuffled.end());
    }
    return result;
}

// Дешифрование
vector<uint32_t> Decrypt(const vector<uint32_t>& encrypted, size_t block_size, const vector<int>& col_key, const vector<int>& row_key){
    vector<uint32_t> result;
    for (size_t start = 0; start < encrypted.size(); start += block_size){
        size_t current_block_size = min(block_size, encrypted.size() - start);
        vector<uint32_t> block(current_block_size, 0);
        for(size_t i = 0; i < current_block_size; ++i){
            block[i] = encrypted[start + i];
        }
        vector<uint32_t> unshuffled = Unshuffle(block, col_key, row_key);
        result.insert(result.end(), unshuffled.begin(), unshuffled.end());
    }
    return result;
}

// Меню и терминальный ввод
void process_terminal_tablet(bool encrypt) {
    size_t block_size;
    cout << "Введите размер блока: ";
    cin >> block_size;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<int> col_key, row_key;
    int k;
    cout << "Введите ключ для столбцов (через пробел, завершите 0): ";
    while (cin >> k && k != 0) col_key.push_back(k);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Введите ключ для строк (через пробел, завершите 0): ";
    while (cin >> k && k != 0) row_key.push_back(k);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Выберите способ работы:\n1 - Через терминал\n2 - Через файл\nВаш выбор: ";
    int mode;
    cin >> mode;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if(mode == 1) {
        cout << "Введите текст: ";
        string input;
        getline(cin, input);
        vector<uint32_t> text_utf32;
        utf8::utf8to32(input.begin(), input.end(), back_inserter(text_utf32));

        vector<uint32_t> result = encrypt ? Encrypt(text_utf32, block_size, col_key, row_key)
                                         : Decrypt(text_utf32, block_size, col_key, row_key);

        string output;
        utf8::utf32to8(result.begin(), result.end(), back_inserter(output));
        cout << (encrypt ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << output << endl;

    } else {
        cout << "Введите имя входного файла: ";
        string inFile;
        getline(cin, inFile);
        string text = readFile(inFile);
        vector<uint32_t> text_utf32;
        utf8::utf8to32(text.begin(), text.end(), back_inserter(text_utf32));

        vector<uint32_t> result = encrypt ? Encrypt(text_utf32, block_size, col_key, row_key)
                                         : Decrypt(text_utf32, block_size, col_key, row_key);

        cout << "Введите имя выходного файла: ";
        string outFile;
        getline(cin, outFile);
        writeFileBinary(outFile + (encrypt ? "_enc" : "_dec"), result);
        cout << "Готово!" << endl;
    }
}

void menu_tablet() {
    while(true) {
        cout << "\nТабличный шифр:\n1 - Шифровать\n2 - Дешифровать\n0 - Назад\nВаш выбор: ";
        int choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if(choice == 0) break;
        if(choice < 1 || choice > 2) { cerr << "Некорректный выбор!\n"; continue; }
        bool encrypt = choice == 1;
        process_terminal_tablet(encrypt);
    }
}