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

// Build encryption and decryption maps for tarabar according to user rules.
static void buildTarabarMaps(unordered_map<uint32_t,uint32_t>& enc, unordered_map<uint32_t,uint32_t>& dec) {
    enc.clear(); dec.clear();

    // --- Russian mappings ---
    // Vowels mapping (lowercase)
    // А Е Ё И О  -> Я Ю Э Ы У
    const vector<pair<uint32_t,uint32_t>> rusVowels = {
        {0x0430, 0x044F}, // а -> я
        {0x0435, 0x044E}, // е -> ю
        {0x0451, 0x044D}, // ё -> э
        {0x0438, 0x044B}, // и -> ы
        {0x043E, 0x0443}  // о -> у
    };
    for (auto &p : rusVowels) {
        uint32_t a = p.first, b = p.second;
        enc[a] = b; dec[b] = a;
        // uppercase
        uint32_t A = (a==0x0451?0x0401:(a - 0x20));
        uint32_t B = (b==0x0451?0x0401:(b - 0x20));
        enc[A] = B; dec[B] = A;
    }

    // Consonant mutual swaps (pairs)
    const vector<pair<uint32_t,uint32_t>> rusConsonantPairs = {
        {0x0431, 0x0449}, // б <-> щ
        {0x0432, 0x0448}, // в <-> ш
        {0x0433, 0x0447}, // г <-> ч
        {0x0434, 0x0446}, // д <-> ц
        {0x0436, 0x0445}, // ж <-> х
        {0x0437, 0x0444}, // з <-> ф
        {0x043A, 0x0442}, // к <-> т
        {0x043B, 0x0441}, // л <-> с
        {0x043C, 0x0440}, // м <-> р
        {0x043D, 0x043F}  // н <-> п
    };
    for (auto &p : rusConsonantPairs) {
        uint32_t l = p.first, r = p.second;
        enc[l] = r; enc[r] = l;
        dec[l] = r; dec[r] = l;
        // uppercase
        uint32_t L = l - 0x20; uint32_t R = r - 0x20;
        enc[L] = R; enc[R] = L;
        dec[L] = R; dec[R] = L;
    }

    // Leave 'й'(0439), 'ъ'(044A), 'ь'(044C) untouched (both cases)

    // --- English mappings (example analogous table) ---
    // Vowels: a e i o u -> u o i e a (reverse)
    const vector<pair<char,char>> engVowels = {{'a','u'},{'e','o'},{'i','i'},{'o','e'},{'u','a'}};
    for (auto &p : engVowels) {
        uint32_t a = (uint32_t)p.first, b = (uint32_t)p.second;
        enc[a] = b; dec[b] = a;
        // uppercase
        enc[a - 0x20] = b - 0x20; dec[b - 0x20] = a - 0x20;
    }

    // Consonant pairs for English (exclude 'y' left unchanged)
    const vector<pair<char,char>> engCons = {
        {'b','n'},{'c','p'},{'d','q'},{'f','r'},{'g','s'},
        {'h','t'},{'j','v'},{'k','w'},{'l','x'},{'m','z'}
    };
    for (auto &p : engCons) {
        uint32_t l = (uint32_t)p.first, r = (uint32_t)p.second;
        enc[l] = r; enc[r] = l; dec[l] = r; dec[r] = l;
        enc[l - 0x20] = r - 0x20; enc[r - 0x20] = l - 0x20;
        dec[l - 0x20] = r - 0x20; dec[r - 0x20] = l - 0x20;
    }
}

// Шифрование текста по таблице замен
vector<unsigned char> encrypt_tarabar(const vector<unsigned char>& input) {
    unordered_map<uint32_t,uint32_t> enc, dec;
    buildTarabarMaps(enc, dec);

    vector<unsigned char> result;
    size_t i = 0;
    while (i < input.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(input, i, bytes);
        uint32_t outcp = cp;
        auto it = enc.find(cp);
        if (it != enc.end()) outcp = it->second;
        cp_to_utf8(outcp, result);
        i += (bytes > 0 ? bytes : 1);
    }
    return result;
}

// Дешифрование текста (так как таблица обратима, используем ту же функцию)
vector<unsigned char> decrypt_tarabar(const vector<unsigned char>& input) {
    unordered_map<uint32_t,uint32_t> enc, dec;
    buildTarabarMaps(enc, dec);

    vector<unsigned char> result;
    size_t i = 0;
    while (i < input.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(input, i, bytes);
        uint32_t outcp = cp;
        auto it = dec.find(cp);
        if (it != dec.end()) outcp = it->second;
        cp_to_utf8(outcp, result);
        i += (bytes > 0 ? bytes : 1);
    }
    return result;
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


