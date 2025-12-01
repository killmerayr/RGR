#pragma once
#include <string>

enum class MenuMode {
    StartMenu,
    EncDecMenu
};

enum class Algorithm {
    MatrixCipher,        // Табличная перестановка
    VigenereCipher,      // Кодовое слово
    TarabarCipher,      // Тарабарская грамота
    ExitProgram
};

enum class CryptoMode {
    Encryption,
    Decryption,
    Cancel
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
void waitForEnter();
int getValidIntInput(int minVal, int maxVal, const std::string& prompt);
