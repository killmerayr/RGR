#include "codeword.h"
#include "../main/utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <fstream>
#include "utf8cpp/utf8.h"

using namespace std;


// Приведение буквы к нижнему регистру для проверки уникальности
uint32_t toLowerForCheck(uint32_t cp) {
    if (cp >= 0x41 && cp <= 0x5A) return cp + 0x20; // A-Z -> a-z
    if (cp >= 0x410 && cp <= 0x42F) return cp + 0x20; // А-Я -> а-я
    return cp;
}

// Получение базовой кодовой точки для сдвига (A/a/А/а)
uint32_t getBaseCodePoint(uint32_t cp) {
    if (cp >= 0x41 && cp <= 0x5A) return 0x41; // A
    if (cp >= 0x61 && cp <= 0x7A) return 0x61; // a
    if (cp >= 0x410 && cp <= 0x42F) return 0x410; // А
    if (cp >= 0x430 && cp <= 0x44F) return 0x430; // а
    return 0;
}

// Получение размера алфавита для кодовой точки
size_t getAlphabetSize(uint32_t cp) {
    if ((cp >= 0x41 && cp <= 0x5A) || (cp >= 0x61 && cp <= 0x7A)) return 26; // латиница
    if ((cp >= 0x410 && cp <= 0x42F) || (cp >= 0x430 && cp <= 0x44F)) return 32; // кириллица
    return 0;
}

// Преобразование строки в вектор кодовых точек UTF-8
vector<uint32_t> stringToCodepoints(const string& str) {
    vector<uint32_t> codepoints;
    auto it = str.begin();
    auto end = str.end();
    while (it != end) {
        uint32_t cp = utf8::next(it, end);
        codepoints.push_back(cp);
    }
    return codepoints;
}

// Преобразование вектора кодовых точек обратно в строку
string codepointsToString(const vector<uint32_t>& codepoints) {
    string result;
    for (uint32_t cp : codepoints) {
        utf8::append(cp, back_inserter(result));
    }
    return result;
}

// Проверка уникальности букв в кодовом слове
bool isValidCodeWord(const vector<uint32_t>& codeWord) {
    unordered_set<uint32_t> seen;
    for (uint32_t cp : codeWord) {
        if (!isAlpha(cp)) return false;
        uint32_t low = toLowerForCheck(cp);
        if (seen.count(low)) return false;
        seen.insert(low);
    }
    return true;
}

// Шифрование текста
string encrypt(const string& text, const string& codeWord) {
    vector<uint32_t> textCP = stringToCodepoints(text);
    vector<uint32_t> codeWordCP = stringToCodepoints(codeWord);
    size_t codeLen = codeWordCP.size();
    vector<uint32_t> result;
    for (size_t i = 0; i < textCP.size(); ++i) {
        uint32_t cp = textCP[i];
        uint32_t key = codeWordCP[i % codeLen];
        if (isAlpha(cp)) {
            uint32_t base = getBaseCodePoint(cp);
            size_t alphSize = getAlphabetSize(cp);
            if (base && alphSize) {
                uint32_t shift = toLowerForCheck(key) - getBaseCodePoint(key);
                uint32_t enc = base + (cp - base + shift) % alphSize;
                result.push_back(enc);
            } else {
                result.push_back(cp);
            }
        } else {
            result.push_back(cp);
        }
    }
    return codepointsToString(result);
}

// Дешифрование текста
string decrypt(const string& text, const string& codeWord) {
    vector<uint32_t> textCP = stringToCodepoints(text);
    vector<uint32_t> codeWordCP = stringToCodepoints(codeWord);
    size_t codeLen = codeWordCP.size();
    vector<uint32_t> result;
    for (size_t i = 0; i < textCP.size(); ++i) {
        uint32_t cp = textCP[i];
        uint32_t key = codeWordCP[i % codeLen];
        if (isAlpha(cp)) {
            uint32_t base = getBaseCodePoint(cp);
            size_t alphSize = getAlphabetSize(cp);
            if (base && alphSize) {
                uint32_t shift = toLowerForCheck(key) - getBaseCodePoint(key);
                uint32_t dec = base + (cp - base + alphSize - shift) % alphSize;
                result.push_back(dec);
            } else {
                result.push_back(cp);
            }
        } else {
            result.push_back(cp);
        }
    }
    return codepointsToString(result);
}


void process_terminal_codeword(bool do_encrypt) {
    cout << "Введите кодовое слово: ";
    string codeWord;
    getline(cin, codeWord);

    if (!isValidCodeWord(stringToCodepoints(codeWord))) {
        cerr << "Некорректное кодовое слово (только уникальные буквы, латиница или кириллица)" << endl;
        return;
    }

    cout << "Введите текст: ";
    string text;
    getline(cin, text);

    string resultText;
    try {
        if (do_encrypt) {
            resultText = encrypt(text, codeWord);
            cout << "Зашифрованный текст:\n" << resultText << endl;
        } else {
            resultText = decrypt(text, codeWord);
            cout << "Дешифрованный текст:\n" << resultText << endl;
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
            cout << "Введите кодовое слово: ";
            string codeWord;
            getline(cin, codeWord);

            if (!isValidCodeWord(stringToCodepoints(codeWord))) {
                cerr << "Некорректное кодовое слово (только уникальные буквы, латиница или кириллица)\n";
                continue;
            }

            if (mode == 1) {
                cout << "Введите текст: ";
                string text;
                getline(cin, text);
                string result = do_encrypt ? encrypt(text, codeWord) : decrypt(text, codeWord);
                cout << (do_encrypt ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << result << endl;
            } else {
                cout << "Введите путь и имя входного файла (путь не должен содержать кирилицу): ";
                string inFile;
                getline(cin, inFile);
                string pass = Password();
                cout << "Введите имя выходного файла: ";
                string outFile;
                getline(cin, outFile);
                string text = readFile(inFile);
                if(do_encrypt){
                    string result = encrypt(text, codeWord);
                    writeFile(outFile + "_enc", result);
                }
                else{
                    string result = decrypt(text, codeWord);
                    writeFile(outFile + "_dec", result);
                }
                cout << "Готово!\n";
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }
}

