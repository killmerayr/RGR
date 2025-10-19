#include "utils.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

bool isAlpha(uint32_t cp) {
    return (cp >= 0x41 && cp <= 0x5A) || // A-Z
           (cp >= 0x61 && cp <= 0x7A) || // a-z
           (cp >= 0x410 && cp <= 0x44F); // А-Яа-я
}

std::string readFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) throw std::runtime_error("Ошибка открытия файла");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    return content;
}

void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) throw std::runtime_error("Ошибка открытия файла для записи");
    out << content;
    out.close();
}
void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    out.close();
}

std::string Password(){
    std:: string password, confirm;
    std::cout << "Введите пароль для файла: ";
    std::getline(std::cin, password);
    std::cout << "Подтвердите пароль: ";
    std::getline(std::cin, confirm);

    if (password != confirm){
        std::cerr << "Пароли не совпадают! Повторите ввод \n";
        return Password();
    }
    return password;
}