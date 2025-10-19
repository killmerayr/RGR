#include <iostream>
#include <string>
#include <stdexcept>
#include "../codeword/codeword.h"
#include "../tablet/tablet.h"
#include "../tarabar/tar.h"

using namespace std;

int main() {
    while (true) {
        cout << "\nВыберите алгоритм:   (алгоритм шифрования должен совпадать с алгоритмом дешифровки)\n"
             << "1 - Шифр с кодовым словом\n"
             << "2 - Табличный шифр\n"
             << "3 - Тарабарский шифр\n"
             << "0 - Выход\n"
             << "Ваш выбор: ";
        int algo;
        cin >> algo;
        cin.ignore();

        if (algo == 0) {
            cout << "Выход.\n";
            break;
        }

        if (algo == 1) {
            menu_codeword();
        } else if (algo == 2) {
            menu_tablet();
        } else if (algo == 3) {
            menu_tarabar();
        } else {
            cerr << "Некорректный выбор алгоритма!\n";
        }
    }
    return 0;
}