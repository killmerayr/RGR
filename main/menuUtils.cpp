#include "menuUtils.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void showMenu(MenuMode mode) {
    clearScreen();
    if (mode == MenuMode::StartMenu) {
        cout << "╔════════════════════════════════════════╗\n";
        cout << "║      Программа шифрования файлов       ║\n";
        cout << "╠════════════════════════════════════════╣\n";
        cout << "║  1. Табличная перестановка             ║\n";
        cout << "║  2. Кодовое слово (Виженер)            ║\n";
        cout << "║  3. Тарабарская грамота                ║\n";
        cout << "║  0. Выход                              ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        cout << "Выберите алгоритм (0-3): ";
    } else if (mode == MenuMode::EncDecMenu) {
        cout << "╔════════════════════════════════════════╗\n";
        cout << "║      Выберите операцию                 ║\n";
        cout << "╠════════════════════════════════════════╣\n";
        cout << "║  1. Шифрование                         ║\n";
        cout << "║  2. Дешифрование                       ║\n";
        cout << "║  0. Отмена                             ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        cout << "Выберите операцию (0-2): ";
    }
}

Algorithm getCryptoAlgorithm() {
    int choice;
    cin >> choice;
    cin.ignore();
    
    switch (choice) {
        case 1: return Algorithm::MatrixCipher;
        case 2: return Algorithm::VigenereCipher;
        case 3: return Algorithm::PlayfairCipher;
        case 0: return Algorithm::ExitProgram;
        default:
            cerr << "Некорректный выбор!\n";
            return Algorithm::ExitProgram;
    }
}

CryptoMode getCryptoMod() {
    int choice;
    cin >> choice;
    cin.ignore();
    
    switch (choice) {
        case 1: return CryptoMode::Encryption;
        case 2: return CryptoMode::Decryption;
        default:
            cerr << "Некорректный выбор!\n";
            return CryptoMode::Encryption;
    }
}

string getFilePath() {
    string filePath;
    cout << "\nВведите путь к файлу: ";
    getline(cin, filePath);
    
    if (!fs::exists(filePath)) {
        cerr << "Файл не найден: " << filePath << endl;
        return "";
    }
    
    return filePath;
}

string createModFile(const string& filePath, 
                    const string& postscript, 
                    CryptoMode action) {
    fs::path p(filePath);
    string filename = p.stem().string();
    string extension = p.extension().string();
    string parent = p.parent_path().string();
    
    if (parent.empty()) {
        parent = ".";
    }
    
    string cryptoFileName = parent + "/" + filename + postscript + extension;
    return cryptoFileName;
}
