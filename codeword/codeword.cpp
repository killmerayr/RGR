#include "codeword.h"
#include "../main/utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <fstream>

using std::vector;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

using namespace std;

// Приведение ASCII буквы к нижнему регистру
unsigned char toLower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

// Проверка кодового слова
bool isValidCodeWord(const vector<unsigned char>& codeWord) {
    unordered_set<unsigned char> seen;
    for (unsigned char c : codeWord) {
        if (!isAsciiLetter(c)) return false;
        unsigned char low = toLower(c);
        if (seen.count(low)) return false;
        seen.insert(low);
    }
    return true;
}

// Шифрование текста
vector<unsigned char> encrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    size_t codeLen = codeWord.size();
    
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = text[i];
        if (!isAsciiLetter(c)) {
            result.push_back(c);
            continue;
        }
        
        unsigned char key = codeWord[i % codeLen];
        bool isUpper = (c >= 'A' && c <= 'Z');
        
        // Приводим к нижнему регистру для шифрования
        c = toLower(c);
        key = toLower(key);
        
        // Шифруем
        unsigned char shift = key - 'a';
        unsigned char enc = 'a' + (c - 'a' + shift) % 26;
        
        // Восстанавливаем регистр если нужно
        if (isUpper) {
            enc = enc - 'a' + 'A';
        }
        
        result.push_back(enc);
    }
    return result;
}

// Дешифрование текста
vector<unsigned char> decrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    size_t codeLen = codeWord.size();
    
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = text[i];
        if (!isAsciiLetter(c)) {
            result.push_back(c);
            continue;
        }
        
        unsigned char key = codeWord[i % codeLen];
        bool isUpper = (c >= 'A' && c <= 'Z');
        
        // Приводим к нижнему регистру для дешифрования
        c = toLower(c);
        key = toLower(key);
        
        // Дешифруем
        unsigned char shift = key - 'a';
        unsigned char dec = 'a' + (c - 'a' + 26 - shift) % 26;
        
        // Восстанавливаем регистр если нужно
        if (isUpper) {
            dec = dec - 'a' + 'A';
        }
        
        result.push_back(dec);
    }
    return result;
}


void process_terminal_codeword(bool do_encrypt) {
    cout << "Введите кодовое слово (только ASCII буквы): ";
    string codeWord;
    getline(cin, codeWord);
    vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

    if (!isValidCodeWord(codeWordBytes)) {
        cerr << "Некорректное кодовое слово (только уникальные ASCII буквы)" << endl;
        return;
    }

    cout << "Введите текст: ";
    string text;
    getline(cin, text);
    vector<unsigned char> textBytes(text.begin(), text.end());

    try {
        vector<unsigned char> resultBytes;
        if (do_encrypt) {
            resultBytes = encrypt(textBytes, codeWordBytes);
            string result(resultBytes.begin(), resultBytes.end());
            cout << "Зашифрованный текст:\n" << result << endl;
        } else {
            resultBytes = decrypt(textBytes, codeWordBytes);
            string result(resultBytes.begin(), resultBytes.end());
            cout << "Дешифрованный текст:\n" << result << endl;
        }
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void menu_codeword() {
    while (true) {
        cout << "\nКодовый шифр:\n"
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
        bool do_encrypt = (action == 1);

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
            cout << "Введите кодовое слово (только ASCII буквы): ";
            string codeWord;
            getline(cin, codeWord);
            vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

            if (!isValidCodeWord(codeWordBytes)) {
                cerr << "Некорректное кодовое слово (только уникальные ASCII буквы)\n";
                continue;
            }

            if (mode == 1) {
                cout << "Введите текст: ";
                string text;
                getline(cin, text);
                vector<unsigned char> textBytes(text.begin(), text.end());
                
                vector<unsigned char> resultBytes = do_encrypt ? 
                    encrypt(textBytes, codeWordBytes) : 
                    decrypt(textBytes, codeWordBytes);
                
                string result(resultBytes.begin(), resultBytes.end());
                cout << (do_encrypt ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << result << endl;
            } else {
                cout << "Введите путь к входному файлу: ";
                string inFile;
                getline(cin, inFile);
                
                cout << "Введите путь к выходному файлу: ";
                string outFile;
                getline(cin, outFile);

                // Читаем файл как бинарный
                vector<unsigned char> fileBytes = readFileBinary(inFile);
                vector<unsigned char> resultBytes = do_encrypt ? 
                    encrypt(fileBytes, codeWordBytes) : 
                    decrypt(fileBytes, codeWordBytes);

                // Записываем результат как бинарный файл
                string outPath = outFile + (do_encrypt ? "_enc" : "_dec");
                writeFileBinary(outPath, resultBytes);
                cout << "Готово!\n";
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }
}

// C-style API для динамической загрузки
extern "C" {
    void codewordEncrypt(const string& inputPath, const string& outputPath) {
        try {
            // Запрашиваем кодовое слово у пользователя
            cout << "Введите кодовое слово (только ASCII буквы): ";
            string codeWord;
            getline(cin, codeWord);
            vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

            if (!isValidCodeWord(codeWordBytes)) {
                cerr << "Ошибка: некорректное кодовое слово" << endl;
                return;
            }

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Шифруем
            vector<unsigned char> encryptedBytes = encrypt(fileBytes, codeWordBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, encryptedBytes);
            
            cout << "Файл зашифрован: " << outputPath << endl;
        } catch (const exception& e) {
            cerr << "Ошибка при шифровании: " << e.what() << endl;
        }
    }

    void codewordDecrypt(const string& inputPath, const string& outputPath) {
        try {
            // Запрашиваем кодовое слово у пользователя
            cout << "Введите кодовое слово (только ASCII буквы): ";
            string codeWord;
            getline(cin, codeWord);
            vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

            if (!isValidCodeWord(codeWordBytes)) {
                cerr << "Ошибка: некорректное кодовое слово" << endl;
                return;
            }

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Дешифруем
            vector<unsigned char> decryptedBytes = decrypt(fileBytes, codeWordBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, decryptedBytes);
            
            cout << "Файл расшифрован: " << outputPath << endl;
        } catch (const exception& e) {
            cerr << "Ошибка при дешифровании: " << e.what() << endl;
        }
    }
}

