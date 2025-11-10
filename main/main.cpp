#include <iostream>
#include <stdexcept>
#include <string>
#include <limits>
#include <dlfcn.h>
#include <clocale>
#include "menuUtils.hpp"

using namespace std;

// Универсальная функция загрузки библиотек
bool processLibrary(const string& libPath,
                    const string& encFunc,
                    const string& decFunc,
                    CryptoMode action,
                    const string& filePath,
                    const string& cryptoFilePath)
{
    void* handler = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handler) {
        cerr << "[Debug] Библиотека не найдена: " << libPath << endl;
        cerr << "Ошибка: " << dlerror() << endl;
        return false;
    }

    typedef void (*CryptoFunc)(const string&, const string&);

    if (action == CryptoMode::Encryption) {
        CryptoFunc encrypt = (CryptoFunc) dlsym(handler, encFunc.c_str());
        if (!encrypt) {
            cerr << "[Debug] Функция не найдена: " << encFunc << endl;
            cerr << "Ошибка: " << dlerror() << endl;
            dlclose(handler);
            return false;
        }
        (*encrypt)(filePath, cryptoFilePath);
        cout << "✓ Файл зашифрован!" << endl;
    } else {
        CryptoFunc decrypt = (CryptoFunc) dlsym(handler, decFunc.c_str());
        if (!decrypt) {
            cerr << "[Debug] Функция не найдена: " << decFunc << endl;
            cerr << "Ошибка: " << dlerror() << endl;
            dlclose(handler);
            return false;
        }
        (*decrypt)(filePath, cryptoFilePath);
        cout << "✓ Файл расшифрован!" << endl;
    }

    dlclose(handler);
    return true;
}

int main() {
    setlocale(LC_ALL, "");
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    clearScreen();

    while (true) {
        showMenu(MenuMode::StartMenu);
        Algorithm userAlgorithm = getCryptoAlgorithm();

        if (userAlgorithm == Algorithm::ExitProgram) {
            clearScreen();
            cout << "До свидания!\n";
            return 0;
        }

        showMenu(MenuMode::EncDecMenu);
        CryptoMode action = getCryptoMod();

        string filePath = getFilePath();
        if (filePath.empty()) {
            cout << "Ошибка при чтении пути файла. Повторите попытку.\n";
            continue;
        }

        string cryptoPostscript = (action == CryptoMode::Encryption) ? "_encrypted" : "_decrypted";
        string cryptoFilePath = createModFile(filePath, cryptoPostscript, action);

        cout << "\nОбработка файла...\n";
        cout << "Входной файл: " << filePath << endl;
        cout << "Выходной файл: " << cryptoFilePath << endl;

        bool success = false;

        switch (userAlgorithm) {
            case Algorithm::MatrixCipher:
                success = processLibrary("./lib/libTabletCipher.so",
                                        "tabletEncrypt", "tabletDecrypt",
                                        action, filePath, cryptoFilePath);
                break;

            case Algorithm::VigenereCipher:
                success = processLibrary("./lib/libCodewordCipher.so",
                                        "codewordEncrypt", "codewordDecrypt",
                                        action, filePath, cryptoFilePath);
                break;

            case Algorithm::PlayfairCipher:
                success = processLibrary("./lib/libTarabarCipher.so",
                                        "tarabarEncrypt", "tarabarDecrypt",
                                        action, filePath, cryptoFilePath);
                break;

            default:
                cerr << "Некорректный выбор алгоритма!" << endl;
                success = false;
                break;
        }

        if (!success) {
            cerr << "\n⚠ Ошибка при обработке файла\n";
        }

        cout << "\n--------------------------------------\n";
    }

    return 0;
}