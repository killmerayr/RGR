#include "tablet.h"
#include "../main/utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>

using namespace std;

// Генерирование случайных ключей
vector<int> generateRandomKey(size_t size) {
    vector<int> key(size);
    for (size_t i = 0; i < size; ++i) {
        key[i] = i;
    }
    
    // Перетасовываем ключ
    random_device rd;
    mt19937 g(rd());
    shuffle(key.begin(), key.end(), g);
    
    return key;
}

// Создание блока из данных
vector<unsigned char> MakeBlock(const vector<unsigned char>& content, size_t start, size_t blocksize) {
    vector<unsigned char> block(blocksize, 0);
    for (size_t i = 0; i < blocksize && start + i < content.size(); ++i) {
        block[i] = content[start + i];
    }
    return block;
}

// Перемешивание блока согласно ключам строк и столбцов
vector<unsigned char> Shuffle(const vector<unsigned char>& block, 
                             const vector<int>& col_key, 
                             const vector<int>& row_key) {
    size_t n = static_cast<size_t>(sqrt(block.size()));
    vector<unsigned char> temp = block;
    vector<unsigned char> new_block(block.size());

    // Перестановка по столбцам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i * n + j < block.size() && i * n + col_key[j] < block.size()) {
                temp[i * n + col_key[j]] = block[i * n + j];
            }
        }
    }

    // Перестановка по строкам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (row_key[i] * n + j < block.size() && i * n + j < block.size()) {
                new_block[row_key[i] * n + j] = temp[i * n + j];
            }
        }
    }

    return new_block;
}

// Обратное перемешивание
vector<unsigned char> Unshuffle(const vector<unsigned char>& block, 
                               const vector<int>& col_key, 
                               const vector<int>& row_key) {
    size_t n = static_cast<size_t>(sqrt(block.size()));
    vector<unsigned char> temp(block.size());
    vector<unsigned char> new_block(block.size());

    // Обратная перестановка по строкам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (row_key[i] * n + j < block.size() && i * n + j < block.size()) {
                temp[i * n + j] = block[row_key[i] * n + j];
            }
        }
    }

    // Обратная перестановка по столбцам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i * n + col_key[j] < block.size() && i * n + j < block.size()) {
                new_block[i * n + j] = temp[i * n + col_key[j]];
            }
        }
    }

    return new_block;
}

// Шифрование с табличной перестановкой
vector<unsigned char> Encrypt(const vector<unsigned char>& text, 
                             size_t block_size, 
                             const vector<int>& col_key, 
                             const vector<int>& row_key) {
    vector<unsigned char> result;
    for (size_t start = 0; start < text.size(); start += block_size) {
        vector<unsigned char> block = MakeBlock(text, start, block_size);
        vector<unsigned char> shuffled = Shuffle(block, col_key, row_key);
        result.insert(result.end(), shuffled.begin(), shuffled.end());
    }
    return result;
}

// Дешифрование с табличной перестановкой
vector<unsigned char> Decrypt(const vector<unsigned char>& encrypted, 
                             size_t block_size, 
                             const vector<int>& col_key, 
                             const vector<int>& row_key) {
    vector<unsigned char> result;
    for (size_t start = 0; start < encrypted.size(); start += block_size) {
        size_t current_block_size = min(block_size, encrypted.size() - start);
        vector<unsigned char> block(current_block_size, 0);
        for (size_t i = 0; i < current_block_size; ++i) {
            block[i] = encrypted[start + i];
        }
        vector<unsigned char> unshuffled = Unshuffle(block, col_key, row_key);
        result.insert(result.end(), unshuffled.begin(), unshuffled.end());
    }
    return result;
}

// Обработка ввода из терминала
void process_terminal_tablet(bool encrypt_flag) {
    size_t block_size;
    cout << "Введите размер блока (квадратный корень должен быть целым): ";
    cin >> block_size;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<int> col_key, row_key;
    size_t n = static_cast<size_t>(sqrt(block_size));

    cout << "Введите ключ для столбцов (индексы 0 до " << (n - 1) << ", завершите -1):\n";
    int k;
    while (cin >> k && k != -1) {
        col_key.push_back(k);
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Введите ключ для строк (индексы 0 до " << (n - 1) << ", завершите -1):\n";
    while (cin >> k && k != -1) {
        row_key.push_back(k);
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (col_key.size() != n || row_key.size() != n) {
        cerr << "Ошибка: размер ключей должен быть " << n << endl;
        return;
    }

    cout << "Введите текст: ";
    string input;
    getline(cin, input);
    vector<unsigned char> text_bytes(input.begin(), input.end());

    vector<unsigned char> result = encrypt_flag ? 
        Encrypt(text_bytes, block_size, col_key, row_key) :
        Decrypt(text_bytes, block_size, col_key, row_key);

    string output(result.begin(), result.end());
    cout << (encrypt_flag ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << output << endl;
}

// Меню
void menu_tablet() {
    while (true) {
        cout << "\nТабличный шифр:\n"
             << "1 - Шифровать\n"
             << "2 - Дешифровать\n"
             << "0 - Назад\n"
             << "Ваш выбор: ";
        int choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) break;
        if (choice < 1 || choice > 2) {
            cerr << "Некорректный выбор!\n";
            continue;
        }

        bool encrypt_flag = (choice == 1);
        
        cout << "Выберите способ работы:\n"
             << "1 - Через терминал\n"
             << "2 - Через файл\n"
             << "0 - Назад\n"
             << "Ваш выбор: ";
        int mode;
        cin >> mode;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (mode == 0) continue;
        if (mode < 1 || mode > 2) {
            cerr << "Некорректный выбор!\n";
            continue;
        }

        try {
            size_t block_size;
            cout << "Введите размер блока (квадратный корень должен быть целым): ";
            cin >> block_size;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            vector<int> col_key, row_key;
            size_t n = static_cast<size_t>(sqrt(block_size));

            cout << "Введите ключ для столбцов (индексы 0 до " << (n - 1) << ", завершите -1):\n";
            int k;
            while (cin >> k && k != -1) {
                col_key.push_back(k);
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Введите ключ для строк (индексы 0 до " << (n - 1) << ", завершите -1):\n";
            while (cin >> k && k != -1) {
                row_key.push_back(k);
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (col_key.size() != n || row_key.size() != n) {
                cerr << "Ошибка: размер ключей должен быть " << n << endl;
                continue;
            }

            if (mode == 1) {
                cout << "Введите текст: ";
                string text;
                getline(cin, text);
                vector<unsigned char> text_bytes(text.begin(), text.end());

                vector<unsigned char> result = encrypt_flag ? 
                    Encrypt(text_bytes, block_size, col_key, row_key) :
                    Decrypt(text_bytes, block_size, col_key, row_key);

                string output(result.begin(), result.end());
                cout << (encrypt_flag ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << output << endl;
            } else {
                cout << "Введите имя входного файла: ";
                string inFile;
                getline(cin, inFile);
                
                cout << "Введите имя выходного файла: ";
                string outFile;
                getline(cin, outFile);

                vector<unsigned char> file_bytes = readFileBinary(inFile);
                vector<unsigned char> result = encrypt_flag ? 
                    Encrypt(file_bytes, block_size, col_key, row_key) :
                    Decrypt(file_bytes, block_size, col_key, row_key);

                string output_path = outFile + (encrypt_flag ? "_enc" : "_dec");
                writeFileBinary(output_path, result);
                cout << "Готово!\n";
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }
}

// C-style API для динамической загрузки
extern "C" {
    void tabletEncrypt(const string& inputPath, const string& outputPath) {
        try {
            // Запрашиваем пароль для защиты файла
            string password = Password();
            
            // Генерируем ключи случайно
            vector<int> col_key = generateRandomKey(TABLET_BLOCK_DIM);
            vector<int> row_key = generateRandomKey(TABLET_BLOCK_DIM);

            cout << "Ключ столбцов: ";
            for (int k : col_key) cout << k << " ";
            cout << endl;

            cout << "Ключ строк: ";
            for (int k : row_key) cout << k << " ";
            cout << endl;

            // Сохраняем ключи и пароль в отдельный файл
            string keyFile = outputPath + ".keys";
            ofstream out(keyFile, ios::binary);
            if (!out.is_open()) {
                cerr << "Ошибка при открытии файла ключей для записи" << endl;
                return;
            }

            // Записываем хеш пароля
            string passHash = hashPassword(password);
            size_t passLen = passHash.length();
            out.write((char*)&passLen, sizeof(passLen));
            out.write(passHash.c_str(), passLen);

            // Записываем размерность (8)
            unsigned char dim = TABLET_BLOCK_DIM;
            out.write((char*)&dim, sizeof(dim));

            // Записываем ключи столбцов
            for (int k : col_key) {
                unsigned char byte = k;
                out.write((char*)&byte, sizeof(byte));
            }

            // Записываем ключи строк
            for (int k : row_key) {
                unsigned char byte = k;
                out.write((char*)&byte, sizeof(byte));
            }
            out.close();

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Шифруем
            vector<unsigned char> encryptedBytes = Encrypt(fileBytes, TABLET_BLOCK_SIZE, col_key, row_key);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, encryptedBytes);
            
            cout << "✓ Файл зашифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при шифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }

    void tabletDecrypt(const string& inputPath, const string& outputPath) {
        try {
            // Проверяем пароль перед расшифровкой
            string keyFile = inputPath + ".keys";
            ifstream keyStream(keyFile, ios::binary);
            if (!keyStream.is_open()) {
                cerr << "Ошибка: не найден файл ключей " << keyFile << endl;
                return;
            }

            // Читаем хеш пароля
            size_t passLen;
            keyStream.read((char*)&passLen, sizeof(passLen));
            string storedHash(passLen, '\0');
            keyStream.read(&storedHash[0], passLen);

            cout << "Введите пароль для файла: ";
            string password;
            getline(cin, password);

            if (!verifyPassword(password, storedHash)) {
                cerr << "Ошибка: неверный пароль!\n";
                keyStream.close();
                return;
            }

            // Запрашиваем ключи у пользователя (или читаем из файла)
            vector<int> col_key, row_key;

            cout << "Введите ключ для столбцов (индексы 0 до " << (TABLET_BLOCK_DIM - 1) << ", завершите -1):\n";
            int k;
            while (cin >> k && k != -1) {
                col_key.push_back(k);
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Введите ключ для строк (индексы 0 до " << (TABLET_BLOCK_DIM - 1) << ", завершите -1):\n";
            while (cin >> k && k != -1) {
                row_key.push_back(k);
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (col_key.size() != TABLET_BLOCK_DIM || row_key.size() != TABLET_BLOCK_DIM) {
                cerr << "Ошибка: размер ключей должен быть " << TABLET_BLOCK_DIM << endl;
                keyStream.close();
                return;
            }

            keyStream.close();

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Дешифруем
            vector<unsigned char> decryptedBytes = Decrypt(fileBytes, TABLET_BLOCK_SIZE, col_key, row_key);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, decryptedBytes);
            
            cout << "✓ Файл расшифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при дешифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }
}