#include "tar.h"
#include "../main/utils.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Создание таблицы замен для ASCII символов
unordered_map<unsigned char, unsigned char> createTarabarMap() {
    unordered_map<unsigned char, unsigned char> m;
    // Английские заглавные буквы (зеркалирование)
    m['A'] = 'Z'; m['Z'] = 'A';
    m['B'] = 'Y'; m['Y'] = 'B';
    m['C'] = 'X'; m['X'] = 'C';
    m['D'] = 'W'; m['W'] = 'D';
    m['E'] = 'V'; m['V'] = 'E';
    m['F'] = 'U'; m['U'] = 'F';
    m['G'] = 'T'; m['T'] = 'G';
    m['H'] = 'S'; m['S'] = 'H';
    m['I'] = 'R'; m['R'] = 'I';
    m['J'] = 'Q'; m['Q'] = 'J';
    m['K'] = 'P'; m['P'] = 'K';
    m['L'] = 'O'; m['O'] = 'L';
    m['M'] = 'N'; m['N'] = 'M';
    // Английские строчные буквы (зеркалирование)
    m['a'] = 'z'; m['z'] = 'a';
    m['b'] = 'y'; m['y'] = 'b';
    m['c'] = 'x'; m['x'] = 'c';
    m['d'] = 'w'; m['w'] = 'd';
    m['e'] = 'v'; m['v'] = 'e';
    m['f'] = 'u'; m['u'] = 'f';
    m['g'] = 't'; m['t'] = 'g';
    m['h'] = 's'; m['s'] = 'h';
    m['i'] = 'r'; m['r'] = 'i';
    m['j'] = 'q'; m['q'] = 'j';
    m['k'] = 'p'; m['p'] = 'k';
    m['l'] = 'o'; m['o'] = 'l';
    m['m'] = 'n'; m['n'] = 'n';
    return m;
}

// Шифрование текста по таблице замен
vector<unsigned char> encrypt_tarabar(const vector<unsigned char>& input) {
    auto table = createTarabarMap();
    vector<unsigned char> result;
    
    for (unsigned char c : input) {
        auto it = table.find(c);
        if (it != table.end()) {
            result.push_back(it->second);
        } else {
            result.push_back(c); // Оставляем символ без изменений, если его нет в таблице
        }
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
            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Шифруем
            vector<unsigned char> encryptedBytes = encrypt_tarabar(fileBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, encryptedBytes);
            
            cout << "Файл зашифрован: " << outputPath << endl;
        } catch (const exception& e) {
            cerr << "Ошибка при шифровании: " << e.what() << endl;
        }
    }

    void tarabarDecrypt(const string& inputPath, const string& outputPath) {
        try {
            // Читаем входной файл
            vector<unsigned char> fileBytes = readFileBinary(inputPath);
            
            // Дешифруем
            vector<unsigned char> decryptedBytes = decrypt_tarabar(fileBytes);
            
            // Записываем выходной файл
            writeFileBinary(outputPath, decryptedBytes);
            
            cout << "Файл расшифрован: " << outputPath << endl;
        } catch (const exception& e) {
            cerr << "Ошибка при дешифровании: " << e.what() << endl;
        }
    }
}


