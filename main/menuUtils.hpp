#pragma once
#include <string>

enum class MenuMode {
    StartMenu,
    EncDecMenu
};

enum class Algorithm {
    MatrixCipher,        // Табличная перестановка
    VigenereCipher,      // Кодовое слово
    PlayfairCipher,      // Тарабарская грамота
    ExitProgram
};

enum class CryptoMode {
    Encryption,
    Decryption
};

// Функции меню
void clearScreen();
void showMenu(MenuMode mode);
Algorithm getCryptoAlgorithm();
CryptoMode getCryptoMod();
std::string getFilePath();
std::string createModFile(const std::string& filePath, 
                         const std::string& postscript, 
                         CryptoMode action);
