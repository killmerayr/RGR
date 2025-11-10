#include "utils.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstring>

bool isAsciiLetter(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// UTF-8 проверка буквы (ASCII или кириллица)
bool isLetter(const std::string& text, size_t pos, size_t& charLen) {
    unsigned char c = text[pos];
    
    // ASCII буква
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        charLen = 1;
        return true;
    }
    
    // UTF-8 кириллица: 2 байта
    // А-Я: 0xD0 0x90 - 0xD0 0xAF
    // а-я: 0xD0 0xB0 - 0xD1 0x8F
    if (pos + 1 < text.length()) {
        unsigned char c1 = text[pos];
        unsigned char c2 = text[pos + 1];
        
        // Кириллица (А-я и ё)
        if ((c1 == 0xD0 && c2 >= 0x90) || (c1 == 0xD1 && c2 <= 0x8F) || 
            (c1 == 0xD0 && c2 == 0xB5) || (c1 == 0xD1 && c2 == 0x91)) {
            charLen = 2;
            return true;
        }
    }
    
    charLen = 1;
    return false;
}

std::vector<unsigned char> readFileBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для чтения: " + filename);
    }
    
    // Читаем файл в вектор байтов
    std::vector<unsigned char> buffer;
    file.seekg(0, std::ios::end);
    std::streamsize length = file.tellg();
    file.seekg(0, std::ios::beg);
    
    buffer.resize(length);
    file.read(reinterpret_cast<char*>(buffer.data()), length);
    
    return buffer;
}

void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

// Обратная совместимость для текстовых файлов
std::string readFile(const std::string& filename) {
    std::vector<unsigned char> buffer = readFileBinary(filename);
    return std::string(buffer.begin(), buffer.end());
}

void writeFile(const std::string& filename, const std::string& content) {
    std::vector<unsigned char> buffer(content.begin(), content.end());
    writeFileBinary(filename, buffer);
}

std::string Password() {
    std::string password, confirm;
    std::cout << "Введите пароль для файла: ";
    std::getline(std::cin, password);
    std::cout << "Подтвердите пароль: ";
    std::getline(std::cin, confirm);

    if (password != confirm) {
        std::cerr << "Пароли не совпадают! Повторите ввод\n";
        return Password();
    }
    return password;
}

// Простое хеширование пароля (XOR-based, для демонстрации)
std::string hashPassword(const std::string& password) {
    // В реальном приложении используйте bcrypt/Argon2
    // Здесь простой вариант для демо
    unsigned char hash[4];
    std::memset(hash, 0, sizeof(hash));
    
    for (size_t i = 0; i < password.length(); ++i) {
        hash[i % 4] ^= password[i];
    }
    
    // Конвертируем в hex строку
    char hexStr[16];
    std::snprintf(hexStr, sizeof(hexStr), "%02x%02x%02x%02x", 
                  hash[0], hash[1], hash[2], hash[3]);
    return std::string(hexStr);
}

bool verifyPassword(const std::string& inputPassword, const std::string& storedHash) {
    return hashPassword(inputPassword) == storedHash;
}