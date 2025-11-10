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

// Forward declarations вспомогательных функций
int getCyrillicIndex(const string& cyrChar);
string indexToCyrillic(int idx);
size_t countCodeWordChars(const vector<unsigned char>& codeWord);
string getCodeWordChar(const vector<unsigned char>& codeWord, size_t charIdx);
int getCharShift(const string& ch);

// Приведение ASCII буквы к нижнему регистру
unsigned char toLower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

// Сохранение пароля в метаданных для выходного файла
void savePasswordToFile(const std::string& filename, const std::string& password) {
    std::string metaFile = filename + ".codeword";
    std::ofstream file(metaFile);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось создать файл с паролем: " + metaFile);
    }
    file << hashPassword(password);
}

// Проверка пароля из метаданных
bool checkPasswordFromFile(const std::string& filename, const std::string& inputPassword) {
    std::string metaFile = filename + ".codeword";
    std::ifstream file(metaFile);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не найден файл с паролем " << metaFile << std::endl;
        return false;
    }
    std::string storedHash;
    std::getline(file, storedHash);
    return verifyPassword(inputPassword, storedHash);
}

// Проверка кодового слова (ASCII и кириллица)
bool isValidCodeWord(const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return false;
    
    // Хотя бы одна буква должна быть
    unordered_set<string> seenChars;
    size_t i = 0;
    
    while (i < codeWord.size()) {
        unsigned char c = codeWord[i];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            string key(1, toLower(c));
            if (seenChars.count(key)) return false;
            seenChars.insert(key);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < codeWord.size()) {
            unsigned char c1 = codeWord[i];
            unsigned char c2 = codeWord[i + 1];
            
            // Проверяем диапазон кириллицы
            if ((c1 == 0xD0 && c2 >= 0x90) || (c1 == 0xD1 && c2 <= 0x8F) || 
                (c1 == 0xD0 && c2 == 0xB5) || (c1 == 0xD1 && c2 == 0x91)) {
                string key(codeWord.begin() + i, codeWord.begin() + i + 2);
                // Приводим к нижнему регистру для кириллицы
                if (key == "\xD0\x81") key = "\xD1\x91"; // Ё -> ё
                if (seenChars.count(key)) return false;
                seenChars.insert(key);
                i += 2;
            } else {
                return false; // Недопустимый символ
            }
        } else {
            return false; // Недопустимый символ
        }
    }
    
    return !seenChars.empty();
}

// Шифрование текста (поддержка ASCII и кириллицы)
vector<unsigned char> encrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    size_t codeIdx = 0;
    size_t textIdx = 0;
    
    while (textIdx < text.size()) {
        unsigned char c = text[textIdx];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            // Получаем ключевой символ из кодового слова
            string keyChar = getCodeWordChar(codeWord, codeIdx);
            int shift = getCharShift(keyChar);
            
            bool isUpper = (c >= 'A' && c <= 'Z');
            c = toLower(c);
            unsigned char enc = 'a' + (c - 'a' + shift) % 26;
            if (isUpper) enc = enc - 'a' + 'A';
            
            result.push_back(enc);
            textIdx++;
            codeIdx = (codeIdx + 1) % countCodeWordChars(codeWord);
        }
        // UTF-8 кириллица (2 байта)
        else if (textIdx + 1 < text.size() && 
                 ((c == 0xD0 && text[textIdx + 1] >= 0x90) || 
                  (c == 0xD1 && text[textIdx + 1] <= 0x8F) || 
                  (c == 0xD0 && text[textIdx + 1] == 0xB5) || 
                  (c == 0xD1 && text[textIdx + 1] == 0x91))) {
            // Получаем ключевой символ
            string keyChar = getCodeWordChar(codeWord, codeIdx);
            int shift = getCharShift(keyChar);
            
            string cyrChar(text.begin() + textIdx, text.begin() + textIdx + 2);
            // Шифруем кириллицу (простой сдвиг в пределах А-Я)
            int cyrIdx = getCyrillicIndex(cyrChar);
            if (cyrIdx >= 0) {
                cyrIdx = (cyrIdx + shift) % 33;
                result.push_back(indexToCyrillic(cyrIdx)[0]);
                result.push_back(indexToCyrillic(cyrIdx)[1]);
            } else {
                result.push_back(text[textIdx]);
                result.push_back(text[textIdx + 1]);
            }
            
            textIdx += 2;
            codeIdx = (codeIdx + 1) % countCodeWordChars(codeWord);
        } else {
            // Не буква — копируем как есть
            result.push_back(c);
            textIdx++;
        }
    }
    
    return result;
}

// Дешифрование текста (поддержка ASCII и кириллицы)
vector<unsigned char> decrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    size_t codeIdx = 0;
    size_t textIdx = 0;
    
    while (textIdx < text.size()) {
        unsigned char c = text[textIdx];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            string keyChar = getCodeWordChar(codeWord, codeIdx);
            int shift = getCharShift(keyChar);
            
            bool isUpper = (c >= 'A' && c <= 'Z');
            c = toLower(c);
            unsigned char dec = 'a' + (c - 'a' + 26 - shift) % 26;
            if (isUpper) dec = dec - 'a' + 'A';
            
            result.push_back(dec);
            textIdx++;
            codeIdx = (codeIdx + 1) % countCodeWordChars(codeWord);
        }
        // UTF-8 кириллица (2 байта)
        else if (textIdx + 1 < text.size() && 
                 ((c == 0xD0 && text[textIdx + 1] >= 0x90) || 
                  (c == 0xD1 && text[textIdx + 1] <= 0x8F) || 
                  (c == 0xD0 && text[textIdx + 1] == 0xB5) || 
                  (c == 0xD1 && text[textIdx + 1] == 0x91))) {
            string keyChar = getCodeWordChar(codeWord, codeIdx);
            int shift = getCharShift(keyChar);
            
            string cyrChar(text.begin() + textIdx, text.begin() + textIdx + 2);
            int cyrIdx = getCyrillicIndex(cyrChar);
            if (cyrIdx >= 0) {
                cyrIdx = (cyrIdx + 33 - shift) % 33;
                result.push_back(indexToCyrillic(cyrIdx)[0]);
                result.push_back(indexToCyrillic(cyrIdx)[1]);
            } else {
                result.push_back(text[textIdx]);
                result.push_back(text[textIdx + 1]);
            }
            
            textIdx += 2;
            codeIdx = (codeIdx + 1) % countCodeWordChars(codeWord);
        } else {
            result.push_back(c);
            textIdx++;
        }
    }
    
    return result;
}

// Вспомогательные функции для работы с кириллицей
int getCyrillicIndex(const string& cyrChar) {
    if (cyrChar.length() != 2) return -1;
    unsigned char c1 = cyrChar[0];
    unsigned char c2 = cyrChar[1];
    
    if (c1 == 0xD0 && c2 >= 0xB0 && c2 <= 0xBF) return c2 - 0xB0; // а-о (0-15)
    if (c1 == 0xD1 && c2 >= 0x80 && c2 <= 0x8F) return 16 + c2 - 0x80; // п-я (16-32)
    if (c1 == 0xD1 && c2 == 0x91) return 32; // ё
    if (c1 == 0xD0 && c2 >= 0x90 && c2 <= 0x9F) return c2 - 0x90 + 33; // А-О (33-48)
    if (c1 == 0xD0 && c2 == 0xB5) return 0; // буква е (в нижнем)
    if (c1 == 0xD0 && c2 == 0x81) return 32; // Ё
    
    return -1;
}

string indexToCyrillic(int idx) {
    if (idx < 0 || idx > 32) return "\xD0\xB0"; // a по умолчанию
    if (idx <= 15) return string(1, 0xD0) + string(1, 0xB0 + idx); // а-о
    if (idx <= 31) return string(1, 0xD1) + string(1, 0x80 + (idx - 16)); // п-я
    return "\xD1\x91"; // ё
}

size_t countCodeWordChars(const vector<unsigned char>& codeWord) {
    size_t count = 0;
    size_t i = 0;
    while (i < codeWord.size()) {
        unsigned char c = codeWord[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            i++;
        } else if (i + 1 < codeWord.size()) {
            i += 2;
        } else {
            i++;
        }
        count++;
    }
    return count > 0 ? count : 1;
}

string getCodeWordChar(const vector<unsigned char>& codeWord, size_t charIdx) {
    size_t count = 0;
    size_t i = 0;
    while (i < codeWord.size() && count <= charIdx) {
        unsigned char c = codeWord[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            if (count == charIdx) return string(1, c);
            i++;
        } else if (i + 1 < codeWord.size()) {
            if (count == charIdx) return string(codeWord.begin() + i, codeWord.begin() + i + 2);
            i += 2;
        } else {
            i++;
        }
        count++;
    }
    return "a";
}

int getCharShift(const string& ch) {
    if (ch.length() == 1) {
        unsigned char c = toLower(ch[0]);
        return c - 'a';
    } else if (ch.length() == 2) {
        return getCyrillicIndex(ch) % 26;
    }
    return 0;
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
            // Запрашиваем кодовое слово
            cout << "Введите кодовое слово (ASCII или русские буквы): ";
            string codeWord;
            getline(cin, codeWord);
            vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

            if (!isValidCodeWord(codeWordBytes)) {
                cerr << "Ошибка: некорректное кодовое слово\n";
                return;
            }

            // Запрашиваем пароль для защиты файла
            string password = Password();
            
            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Шифруем
            vector<unsigned char> encryptedBytes = encrypt(fileBytes, codeWordBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, encryptedBytes);
            
            // Сохраняем хеш пароля в метаданные
            savePasswordToFile(outputPath, password);
            
            cout << "✓ Файл зашифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при шифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }

    void codewordDecrypt(const string& inputPath, const string& outputPath) {
        try {
            // Проверяем пароль перед расшифровкой
            cout << "Введите пароль для файла: ";
            string password;
            getline(cin, password);
            
            if (!checkPasswordFromFile(inputPath, password)) {
                cerr << "Ошибка: неверный пароль!\n";
                return;
            }
            
            // Запрашиваем кодовое слово
            cout << "Введите кодовое слово (ASCII или русские буквы): ";
            string codeWord;
            getline(cin, codeWord);
            vector<unsigned char> codeWordBytes(codeWord.begin(), codeWord.end());

            if (!isValidCodeWord(codeWordBytes)) {
                cerr << "Ошибка: некорректное кодовое слово\n";
                return;
            }

            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Дешифруем
            vector<unsigned char> decryptedBytes = decrypt(fileBytes, codeWordBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, decryptedBytes);
            
            cout << "✓ Файл расшифрован!\n";
        } catch (const exception& e) {
            cerr << "Ошибка при дешифровании: " << e.what() << endl;
            remove(outputPath.c_str());
        }
    }
}

