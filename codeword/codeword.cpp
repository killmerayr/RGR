#include "codeword.h"
#include "../main/utils.h"
#include <iostream>
#include <string>
#include <cstdint>
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
// UTF-8 / codepoint helpers and table builders
uint32_t utf8_to_cp(const string& s, size_t pos, size_t& bytes);
string cp_to_utf8(uint32_t cp);
uint32_t toLowerCp(uint32_t cp);
int cyrillicCpToIndex(uint32_t cp);
uint32_t cyrillicIndexToCp(int idx);

// Substitution tables use code points (uint32_t) rather than hard-coded arrays
vector<uint32_t> createSubstitutionTableEnglish(const string& codeWord);
vector<uint32_t> createSubstitutionTableCyrillic(const string& codeWord);
uint32_t encryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table);
uint32_t decryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table);
uint32_t encryptCharCyrillicCp(uint32_t cp, const vector<uint32_t>& table);
uint32_t decryptCharCyrillicCp(uint32_t cp, const vector<uint32_t>& table);

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
vector<uint32_t> createSubstitutionTableEnglish(const string& codeWord) {
    vector<uint32_t> table;
    unordered_set<uint32_t> used;

    // Пробегаем по кодовому слову, декодируем UTF-8, добавляем английские буквы в lower-case
    size_t i = 0;
    while (i < codeWord.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(codeWord, i, bytes);
        if (bytes == 1) {
            // ASCII
            uint32_t lower = toLower((unsigned char)cp);
            if (lower >= 'a' && lower <= 'z' && !used.count(lower)) {
                used.insert(lower);
                table.push_back(lower);
            }
        }
        i += (bytes > 0 ? bytes : 1);
    }

    // Добавляем оставшиеся буквы по кодовым значениям
    for (uint32_t cp = (uint32_t)'a'; cp <= (uint32_t)'z'; ++cp) {
        if (!used.count(cp)) table.push_back(cp);
    }

    return table;
}

// Создание таблицы подстановки для кириллицы
// Пример: кодовое слово "мир" для кириллицы: ["м", "и", "р", "а", "б", "в", ..., "я"]
vector<uint32_t> createSubstitutionTableCyrillic(const string& codeWord) {
    vector<uint32_t> table;
    unordered_set<uint32_t> used;

    // Проходим кодовое слово, добавляем русские буквы (нормализованные в lower-case)
    size_t i = 0;
    while (i < codeWord.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(codeWord, i, bytes);
        if (bytes == 2) {
            uint32_t lower = toLowerCp(cp);
            int idx = cyrillicCpToIndex(lower);
            if (idx >= 0) {
                if (!used.count(lower)) {
                    used.insert(lower);
                    table.push_back(lower);
                }
            }
        }
        i += (bytes > 0 ? bytes : 1);
    }

    // Добавляем оставшиеся кириллические буквы по индексам (а..я с ё после е)
    for (int idx = 0; idx <= 32; ++idx) {
        uint32_t cp = cyrillicIndexToCp(idx);
        if (!used.count(cp)) {
            table.push_back(cp);
        }
    }

    return table;
}

// Шифрование одной ASCII буквы
uint32_t encryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table) {
    bool isUpper = (cp >= 'A' && cp <= 'Z');
    uint32_t lower = toLower((unsigned char)cp);
    int idx = (int)(lower - 'a');
    if (idx < 0 || idx >= (int)table.size()) return cp;
    uint32_t encrypted = table[idx];
    if (isUpper) {
        // make uppercase
        return (uint32_t)((char)(encrypted - 'a' + 'A'));
    }
    return encrypted;
}

uint32_t decryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table) {
    bool isUpper = (cp >= 'A' && cp <= 'Z');
    uint32_t lower = toLower((unsigned char)cp);
    // find in table
    for (int i = 0; i < (int)table.size(); ++i) {
        if (table[i] == lower) {
            uint32_t dec = (uint32_t)('a' + i);
            if (isUpper) return (uint32_t)((char)(dec - 'a' + 'A'));
            return dec;
        }
    }
    return cp;
}

// Шифрование одного кириллицы символа
uint32_t encryptCharCyrillicCp(uint32_t cp, const vector<uint32_t>& table) {
    // remember case
    bool isUpper = false;
    uint32_t orig = cp;
    // uppercase Cyrillic range U+0410..U+042F and U+0401 (Ё)
    if ((cp >= 0x0410 && cp <= 0x042F) || cp == 0x0401) {
        isUpper = true;
        // convert to lowercase
        if (cp == 0x0401) cp = 0x0451; else cp = cp + 0x20;
    }

    // normalize ё uppercase handled above
    uint32_t lower = toLowerCp(cp);
    int idx = cyrillicCpToIndex(lower);
    if (idx < 0 || idx >= (int)table.size()) return orig;
    uint32_t mapped = table[idx];
    // restore case
    if (isUpper) {
        if (mapped == 0x0451) return 0x0401;
        return mapped - 0x20;
    }
    return mapped;
}

uint32_t decryptCharCyrillicCp(uint32_t cp, const vector<uint32_t>& table) {
    bool isUpper = false;
    uint32_t orig = cp;
    if ((cp >= 0x0410 && cp <= 0x042F) || cp == 0x0401) {
        isUpper = true;
        if (cp == 0x0401) cp = 0x0451; else cp = cp + 0x20;
    }
    uint32_t lower = toLowerCp(cp);
    // find in table
    int found = -1;
    for (int i = 0; i < (int)table.size(); ++i) {
        if (table[i] == lower) { found = i; break; }
    }
    if (found < 0) return orig;
    uint32_t dec = cyrillicIndexToCp(found);
    if (isUpper) {
        if (dec == 0x0451) return 0x0401;
        return dec - 0x20;
    }
    return dec;
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
    
    vector<uint32_t> tableEnglish;
    vector<uint32_t> tableCyrillic;
    string textStr(text.begin(), text.end());
    
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
            uint32_t cp = (unsigned char)c;
            uint32_t mapped = encryptCharEnglishCp(cp, tableEnglish);
            result.push_back((unsigned char)mapped);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < text.size() && 
                 ((c == 0xD0 && ((text[i+1] >= 0x90 && text[i+1] <= 0xBF) || text[i+1] == 0x81)) ||
                  (c == 0xD1 && ((text[i+1] >= 0x80 && text[i+1] <= 0x8F) || text[i+1] == 0x91)))) {
            size_t bytes = 0;
            uint32_t cp = utf8_to_cp(textStr, i, bytes);
            uint32_t mapped = encryptCharCyrillicCp(cp, tableCyrillic);
            string enc = cp_to_utf8(mapped);
            for (unsigned char ch : enc) result.push_back(ch);
            i += bytes > 0 ? bytes : 2;
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
    
    vector<uint32_t> tableEnglish = createSubstitutionTableEnglish(codeWordStr);
    vector<uint32_t> tableCyrillic = createSubstitutionTableCyrillic(codeWordStr);
    string textStr(text.begin(), text.end());
    
    // Дешифруем текст
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = text[i];
        
        // ASCII буква
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            uint32_t cp = (unsigned char)c;
            uint32_t mapped = decryptCharEnglishCp(cp, tableEnglish);
            result.push_back((unsigned char)mapped);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < text.size() && 
                 ((c == 0xD0 && ((text[i+1] >= 0x90 && text[i+1] <= 0xBF) || text[i+1] == 0x81)) ||
                  (c == 0xD1 && ((text[i+1] >= 0x80 && text[i+1] <= 0x8F) || text[i+1] == 0x91)))) {
            size_t bytes = 0;
            uint32_t cp = utf8_to_cp(textStr, i, bytes);
            uint32_t mapped = decryptCharCyrillicCp(cp, tableCyrillic);
            string dec = cp_to_utf8(mapped);
            for (unsigned char ch : dec) result.push_back(ch);
            i += bytes > 0 ? bytes : 2;
        } else {
            // Не буква — копируем как есть
            result.push_back(c);
            i++;
        }
    }
    
    return result;
}


// Вспомогательные функции для работы с UTF-8 и кодовыми значениями
// Декодируем кодовую точку UTF-8 из строки s, начиная с pos.
// Возвращаем кодовую точку и количество байт в bytes (1 или 2 for our use).
uint32_t utf8_to_cp(const string& s, size_t pos, size_t& bytes) {
    bytes = 0;
    if (pos >= s.size()) return 0;
    unsigned char c0 = (unsigned char)s[pos];
    if (c0 < 0x80) {
        bytes = 1;
        return (uint32_t)c0;
    }
    // expecting 2-byte Cyrillic
    if (pos + 1 < s.size()) {
        unsigned char c1 = (unsigned char)s[pos+1];
        bytes = 2;
        // decode two-byte UTF-8
        uint32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        return cp;
    }
    // fallback
    bytes = 1;
    return (uint32_t)c0;
}

// Кодовая точка в UTF-8 строку
string cp_to_utf8(uint32_t cp) {
    if (cp < 0x80) {
        return string(1, (char)cp);
    }
    if (cp <= 0x7FF) {
        unsigned char b1 = 0xC0 | ((cp >> 6) & 0x1F);
        unsigned char b2 = 0x80 | (cp & 0x3F);
        return string((char*)"", 0) + (char)b1 + (char)b2;
    }
    // not expected for our alphabets, but handle 3-byte
    unsigned char b1 = 0xE0 | ((cp >> 12) & 0x0F);
    unsigned char b2 = 0x80 | ((cp >> 6) & 0x3F);
    unsigned char b3 = 0x80 | (cp & 0x3F);
    string s;
    s.push_back((char)b1);
    s.push_back((char)b2);
    s.push_back((char)b3);
    return s;
}

// Нормализация в нижний регистр для ASCII и кириллицы (codepoints)
uint32_t toLowerCp(uint32_t cp) {
    // ASCII
    if (cp >= 'A' && cp <= 'Z') return cp + 0x20;
    // Cyrillic uppercase А..Я U+0410..U+042F -> +0x20 to get lowercase
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;
    // Ё (uppercase U+0401) -> ё U+0451
    if (cp == 0x0401) return 0x0451;
    return cp;
}

// Преобразование кодовой точки кириллицы в индекс 0..32 (а..я с ё после е)
int cyrillicCpToIndex(uint32_t cp) {
    // expecting lowercase codepoint
    if (cp == 0x0451) return 6; // ё
    if (cp >= 0x0430 && cp <= 0x0435) return cp - 0x0430; // а..е -> 0..5
    if (cp >= 0x0436 && cp <= 0x044F) return 7 + (cp - 0x0436); // ж..я -> 7..
    return -1;
}

// Индекс 0..32 -> кодовая точка (lowercase)
uint32_t cyrillicIndexToCp(int idx) {
    if (idx < 0 || idx > 32) return 0x0430;
    if (idx == 6) return 0x0451;
    if (idx <= 5) return 0x0430 + idx;
    // idx >=7
    return 0x0436 + (idx - 7);
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

