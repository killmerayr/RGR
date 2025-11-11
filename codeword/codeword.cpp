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
vector<char> createSubstitutionTableEnglish(const string& codeWord);
vector<string> createSubstitutionTableCyrillic(const string& codeWord);
char encryptCharEnglish(char c, const vector<char>& table);
char decryptCharEnglish(char c, const vector<char>& table);
string encryptCharCyrillic(const string& cyrChar, const vector<string>& table);
string decryptCharCyrillic(const string& cyrChar, const vector<string>& table);

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
    
    // Проверка на уникальные буквы
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
        else if (i + 1 < codeWord.size() && 
                 ((c == 0xD0 && ((codeWord[i+1] >= 0x90 && codeWord[i+1] <= 0xBF) || codeWord[i+1] == 0x81)) ||
                  (c == 0xD1 && ((codeWord[i+1] >= 0x80 && codeWord[i+1] <= 0x8F) || codeWord[i+1] == 0x91)))) {
            string key(codeWord.begin() + i, codeWord.begin() + i + 2);
            if (key == "\xD0\x81") key = "\xD1\x91"; // Ё -> ё
            if (seenChars.count(key)) return false;
            seenChars.insert(key);
            i += 2;
        } else {
            return false;
        }
    }
    
    return !seenChars.empty();
}

// Создание таблицы подстановки для английского алфавита
// Пример: кодовое слово "мир" -> таблица: ["м", "и", "р", "а", "б", "в", ..., "я"]
vector<char> createSubstitutionTableEnglish(const string& codeWord) {
    vector<char> table;
    unordered_set<char> used;
    
    // Добавляем уникальные буквы из кодового слова
    for (char c : codeWord) {
        char lower = toLower(c);
        if ((lower >= 'a' && lower <= 'z') && !used.count(lower)) {
            table.push_back(lower);
            used.insert(lower);
        }
    }
    
    // Добавляем оставшиеся буквы в алфавитном порядке
    for (char c = 'a'; c <= 'z'; c++) {
        if (!used.count(c)) {
            table.push_back(c);
        }
    }
    
    return table;
}

// Создание таблицы подстановки для кириллицы
// Пример: кодовое слово "мир" для кириллицы: ["м", "и", "р", "а", "б", "в", ..., "я"]
vector<string> createSubstitutionTableCyrillic(const string& codeWord) {
    vector<string> table;
    unordered_set<string> used;
    
    // Русский алфавит: а-я + ё
    const vector<string> cyrAlphabet = {
        "\xD0\xB0", "\xD0\xB1", "\xD0\xB2", "\xD0\xB3", "\xD0\xB4", "\xD0\xB5",
        "\xD1\x91", "\xD0\xB6", "\xD0\xB7", "\xD0\xB8", "\xD0\xB9", "\xD0\xBA",
        "\xD0\xBB", "\xD0\xBC", "\xD0\xBD", "\xD0\xBE", "\xD0\xBF", "\xD1\x80",
        "\xD1\x81", "\xD1\x82", "\xD1\x83", "\xD1\x84", "\xD1\x85", "\xD1\x86",
        "\xD1\x87", "\xD1\x88", "\xD1\x89", "\xD1\x8A", "\xD1\x8B", "\xD1\x8C",
        "\xD1\x8D", "\xD1\x8E", "\xD1\x8F"
    };
    
    // Добавляем уникальные буквы из кодового слова в нижнем регистре
    size_t i = 0;
    while (i < codeWord.size()) {
        unsigned char c = codeWord[i];
        
        // ASCII буква - пропускаем для кириллицы
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            i++;
            continue;
        }
        
        // UTF-8 кириллица (2 байта)
        if (i + 1 < codeWord.size()) {
            string cyrChar(codeWord.begin() + i, codeWord.begin() + i + 2);
            
            // Приводим к нижнему регистру
            if (cyrChar == "\xD0\x81") cyrChar = "\xD1\x91"; // Ё -> ё
            
            if (!used.count(cyrChar)) {
                table.push_back(cyrChar);
                used.insert(cyrChar);
            }
            i += 2;
        } else {
            i++;
        }
    }
    
    // Добавляем оставшиеся буквы
    for (const auto& ch : cyrAlphabet) {
        if (!used.count(ch)) {
            table.push_back(ch);
        }
    }
    
    return table;
}

// Шифрование одной ASCII буквы
char encryptCharEnglish(char c, const vector<char>& table) {
    bool isUpper = (c >= 'A' && c <= 'Z');
    c = toLower(c);
    
    int idx = c - 'a';
    if (idx < 0 || idx >= table.size()) return c;
    
    char encrypted = table[idx];
    return isUpper ? (char)(encrypted - 'a' + 'A') : encrypted;
}

// Дешифрование одной ASCII буквы
char decryptCharEnglish(char c, const vector<char>& table) {
    bool isUpper = (c >= 'A' && c <= 'Z');
    c = toLower(c);
    
    // Ищем букву в таблице и возвращаем её индекс (букву стандартного алфавита)
    for (int i = 0; i < table.size(); i++) {
        if (table[i] == c) {
            char decrypted = 'a' + i;
            return isUpper ? (char)(decrypted - 'a' + 'A') : decrypted;
        }
    }
    return c;
}

// Шифрование одного кириллицы символа
string encryptCharCyrillic(const string& cyrChar, const vector<string>& table) {
    // Приводим к нижнему регистру
    string lower = cyrChar;
    if (lower == "\xD0\x81") lower = "\xD1\x91"; // Ё -> ё
    
    // Ищем индекс в стандартном алфавите
    const vector<string> cyrAlphabet = {
        "\xD0\xB0", "\xD0\xB1", "\xD0\xB2", "\xD0\xB3", "\xD0\xB4", "\xD0\xB5",
        "\xD1\x91", "\xD0\xB6", "\xD0\xB7", "\xD0\xB8", "\xD0\xB9", "\xD0\xBA",
        "\xD0\xBB", "\xD0\xBC", "\xD0\xBD", "\xD0\xBE", "\xD0\xBF", "\xD1\x80",
        "\xD1\x81", "\xD1\x82", "\xD1\x83", "\xD1\x84", "\xD1\x85", "\xD1\x86",
        "\xD1\x87", "\xD1\x88", "\xD1\x89", "\xD1\x8A", "\xD1\x8B", "\xD1\x8C",
        "\xD1\x8D", "\xD1\x8E", "\xD1\x8F"
    };
    
    int idx = -1;
    for (int i = 0; i < cyrAlphabet.size(); i++) {
        if (cyrAlphabet[i] == lower) {
            idx = i;
            break;
        }
    }
    
    if (idx < 0 || idx >= table.size()) return cyrChar;
    
    return table[idx];
}

// Дешифрование одного кириллицы символа
string decryptCharCyrillic(const string& cyrChar, const vector<string>& table) {
    const vector<string> cyrAlphabet = {
        "\xD0\xB0", "\xD0\xB1", "\xD0\xB2", "\xD0\xB3", "\xD0\xB4", "\xD0\xB5",
        "\xD1\x91", "\xD0\xB6", "\xD0\xB7", "\xD0\xB8", "\xD0\xB9", "\xD0\xBA",
        "\xD0\xBB", "\xD0\xBC", "\xD0\xBD", "\xD0\xBE", "\xD0\xBF", "\xD1\x80",
        "\xD1\x81", "\xD1\x82", "\xD1\x83", "\xD1\x84", "\xD1\x85", "\xD1\x86",
        "\xD1\x87", "\xD1\x88", "\xD1\x89", "\xD1\x8A", "\xD1\x8B", "\xD1\x8C",
        "\xD1\x8D", "\xD1\x8E", "\xD1\x8F"
    };
    
    // Ищем букву в таблице и возвращаем её стандартное положение
    for (int i = 0; i < table.size(); i++) {
        if (table[i] == cyrChar) {
            return cyrAlphabet[i];
        }
    }
    return cyrChar;
}

// Шифрование текста (поддержка ASCII и кириллицы)
vector<unsigned char> encrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    
    // Определяем язык и создаём таблицы подстановки
    bool hasEnglish = false, hasCyrillic = false;
    string codeWordStr(codeWord.begin(), codeWord.end());
    
    // Проверяем язык текста
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = text[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            hasEnglish = true;
            i++;
        } else if (i + 1 < text.size() && 
                   ((c == 0xD0 && ((text[i+1] >= 0x90 && text[i+1] <= 0xBF) || text[i+1] == 0x81)) ||
                    (c == 0xD1 && ((text[i+1] >= 0x80 && text[i+1] <= 0x8F) || text[i+1] == 0x91)))) {
            hasCyrillic = true;
            i += 2;
        } else {
            i++;
        }
    }
    
    vector<char> tableEnglish;
    vector<string> tableCyrillic;
    
    if (hasEnglish) {
        tableEnglish = createSubstitutionTableEnglish(codeWordStr);
    }
    if (hasCyrillic) {
        tableCyrillic = createSubstitutionTableCyrillic(codeWordStr);
    }
    
    // Шифруем текст
    i = 0;
    while (i < text.size()) {
        unsigned char c = text[i];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            char encrypted = encryptCharEnglish(c, tableEnglish);
            result.push_back(encrypted);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < text.size() && 
                 ((c == 0xD0 && ((text[i+1] >= 0x90 && text[i+1] <= 0xBF) || text[i+1] == 0x81)) ||
                  (c == 0xD1 && ((text[i+1] >= 0x80 && text[i+1] <= 0x8F) || text[i+1] == 0x91)))) {
            string cyrChar(text.begin() + i, text.begin() + i + 2);
            string encrypted = encryptCharCyrillic(cyrChar, tableCyrillic);
            for (char ch : encrypted) {
                result.push_back((unsigned char)ch);
            }
            i += 2;
        } else {
            // Не буква — копируем как есть
            result.push_back(c);
            i++;
        }
    }
    
    return result;
}

// Дешифрование текста (поддержка ASCII и кириллицы)
vector<unsigned char> decrypt(const vector<unsigned char>& text, const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return text;
    
    vector<unsigned char> result;
    string codeWordStr(codeWord.begin(), codeWord.end());
    
    vector<char> tableEnglish = createSubstitutionTableEnglish(codeWordStr);
    vector<string> tableCyrillic = createSubstitutionTableCyrillic(codeWordStr);
    
    // Дешифруем текст
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = text[i];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            char decrypted = decryptCharEnglish(c, tableEnglish);
            result.push_back(decrypted);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < text.size() && 
                 ((c == 0xD0 && ((text[i+1] >= 0x90 && text[i+1] <= 0xBF) || text[i+1] == 0x81)) ||
                  (c == 0xD1 && ((text[i+1] >= 0x80 && text[i+1] <= 0x8F) || text[i+1] == 0x91)))) {
            string cyrChar(text.begin() + i, text.begin() + i + 2);
            string decrypted = decryptCharCyrillic(cyrChar, tableCyrillic);
            for (char ch : decrypted) {
                result.push_back((unsigned char)ch);
            }
            i += 2;
        } else {
            // Не буква — копируем как есть
            result.push_back(c);
            i++;
        }
    }
    
    return result;
}

// Вспомогательные функции для работы с кириллицей



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

