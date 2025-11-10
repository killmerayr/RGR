#include "utils.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vector>

bool isAsciiLetter(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
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