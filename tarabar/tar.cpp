#include "tar.h"
#include "../main/utils.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cstdint>

using namespace std;

// Создание таблицы замен для ASCII символов
// Helpers: UTF-8 decode/encode for 1-2 byte sequences (ASCII and Cyrillic)
static uint32_t utf8_to_cp(const vector<unsigned char>& data, size_t pos, size_t& bytes) {
    bytes = 0;
    if (pos >= data.size()) return 0;
    unsigned char c0 = data[pos];
    if (c0 < 0x80) { bytes = 1; return (uint32_t)c0; }
    if (pos + 1 < data.size()) {
        unsigned char c1 = data[pos+1];
        bytes = 2;
        uint32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        return cp;
    }
    bytes = 1; return (uint32_t)c0;
}

static void cp_to_utf8(uint32_t cp, vector<unsigned char>& out) {
    if (cp < 0x80) { out.push_back((unsigned char)cp); return; }
    if (cp <= 0x7FF) {
        unsigned char b1 = 0xC0 | ((cp >> 6) & 0x1F);
        unsigned char b2 = 0x80 | (cp & 0x3F);
        out.push_back(b1); out.push_back(b2); return;
    }
    unsigned char b1 = 0xE0 | ((cp >> 12) & 0x0F);
    unsigned char b2 = 0x80 | ((cp >> 6) & 0x3F);
    unsigned char b3 = 0x80 | (cp & 0x3F);
    out.push_back(b1); out.push_back(b2); out.push_back(b3);
}

static uint32_t toLowerCp(uint32_t cp) {
    if (cp >= 'A' && cp <= 'Z') return cp + 0x20;
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;
    if (cp == 0x0401) return 0x0451;
    return cp;
}

static int cyrillicCpToIndex(uint32_t cp) {
    if (cp == 0x0451) return 6;
    if (cp >= 0x0430 && cp <= 0x0435) return cp - 0x0430;
    if (cp >= 0x0436 && cp <= 0x044F) return 7 + (cp - 0x0436);
    return -1;
}

static uint32_t cyrillicIndexToCp(int idx) {
    if (idx == 6) return 0x0451;
    if (idx <= 5) return 0x0430 + idx;
    return 0x0436 + (idx - 7);
}

// Map a single codepoint using mirror transformation within alphabet ranges
static uint32_t mirrorMapCp(uint32_t cp) {
    // ASCII uppercase
    if (cp >= 'A' && cp <= 'Z') return (uint32_t)('A' + ('Z' - cp));
    // ASCII lowercase
    if (cp >= 'a' && cp <= 'z') return (uint32_t)('a' + ('z' - cp));

    // Cyrillic
    uint32_t orig = cp;
    bool isUpper = false;
    if ((cp >= 0x0410 && cp <= 0x042F) || cp == 0x0401) {
        isUpper = true;
        if (cp == 0x0401) cp = 0x0451; else cp = cp + 0x20;
    }
    uint32_t lower = toLowerCp(cp);
    int idx = cyrillicCpToIndex(lower);
    if (idx >= 0) {
        int mirrored = 32 - idx;
        uint32_t mapped = cyrillicIndexToCp(mirrored);
        if (isUpper) {
            if (mapped == 0x0451) return 0x0401;
            return mapped - 0x20;
        }
        return mapped;
    }
    return orig;
}

// Шифрование текста по таблице замен
vector<unsigned char> encrypt_tarabar(const vector<unsigned char>& input) {
    vector<unsigned char> result;
    size_t i = 0;
    while (i < input.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(input, i, bytes);
        uint32_t mapped = mirrorMapCp(cp);
        cp_to_utf8(mapped, result);
        i += (bytes > 0 ? bytes : 1);
    }
    return result;
}

// Дешифрование текста (так как таблица обратима, используем ту же функцию)
vector<unsigned char> decrypt_tarabar(const vector<unsigned char>& input) {
    return encrypt_tarabar(input);
}

// Обработка ввода из терминала
void process_terminal_tarabar(bool encrypt_flag) {
    cout << "Введите текст: ";
    string text;
    getline(cin, text);
    
    vector<unsigned char> input_bytes(text.begin(), text.end());
    
    try {
        vector<unsigned char> result = encrypt_flag ? 
            encrypt_tarabar(input_bytes) : 
            decrypt_tarabar(input_bytes);
        
        string output(result.begin(), result.end());
        cout << (encrypt_flag ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << output << endl;
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

// Главное меню
void menu_tarabar() {
    while (true) {
        cout << "\nТарабарский шифр (ASCII версия):\n"
             << "1 - Шифровать\n"
             << "2 - Дешифровать\n"
             << "0 - Назад\n"
             << "Ваш выбор: ";
        int action;
        cin >> action;
        cin.ignore();

        if (action == 0) break;
        if (action < 1 || action > 2) {
            cerr << "Некорректный выбор!\n";
            continue;
        }
        
        bool encrypt_flag = (action == 1);

        cout << "Выберите способ работы:\n"
             << "1 - Через терминал\n"
             << "2 - Через файл\n"
             << "0 - Назад\n"
             << "Ваш выбор: ";
        int mode;
        cin >> mode;
        cin.ignore();

        if (mode == 0) continue;
        if (mode < 1 || mode > 2) {
            cerr << "Некорректный выбор!\n";
            continue;
        }

        try {
            if (mode == 1) {
                cout << "Введите текст: ";
                string text;
                getline(cin, text);
                vector<unsigned char> input_bytes(text.begin(), text.end());
                
                vector<unsigned char> result = encrypt_flag ? 
                    encrypt_tarabar(input_bytes) : 
                    decrypt_tarabar(input_bytes);
                
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
                    encrypt_tarabar(file_bytes) : 
                    decrypt_tarabar(file_bytes);
                
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
    void tarabarEncrypt(const string& inputPath, const string& outputPath) {
        try {
            // Запрашиваем пароль для защиты файла
            string password = Password();
            
            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Шифруем
            vector<unsigned char> encryptedBytes = encrypt_tarabar(fileBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, encryptedBytes);
            
            // Сохраняем хеш пароля в метаданные
            string metaFile = outputPath + ".tarabar";
            ofstream meta(metaFile);
            if (!meta.is_open()) {
                cerr << "Ошибка при сохранении пароля" << endl;
                return;
            }
            meta << hashPassword(password);
            meta.close();
            
            cout << "✓ Файл зашифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при шифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }

    void tarabarDecrypt(const string& inputPath, const string& outputPath) {
        try {
            // Проверяем пароль перед расшифровкой
            string metaFile = inputPath + ".tarabar";
            ifstream meta(metaFile);
            if (!meta.is_open()) {
                cerr << "Ошибка: не найден файл с паролем " << metaFile << endl;
                return;
            }
            string storedHash;
            getline(meta, storedHash);
            meta.close();

            cout << "Введите пароль для файла: ";
            string password;
            getline(cin, password);

            if (!verifyPassword(password, storedHash)) {
                cerr << "Ошибка: неверный пароль!\n";
                return;
            }

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Дешифруем
            vector<unsigned char> decryptedBytes = decrypt_tarabar(fileBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, decryptedBytes);
            
            cout << "✓ Файл расшифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при дешифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }
}


