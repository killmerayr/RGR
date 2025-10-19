#include "tar.h"
#include "../main/utils.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include "utf8cpp/utf8.h"

using namespace std;


vector<uint32_t> utf8_to_codepoints(const string &str) {
    vector<uint32_t> result;
    auto it = str.begin();
    while (it != str.end()) {
        uint32_t cp = utf8::next(it, str.end());
        result.push_back(cp);
    }
    return result;
}

string codepoints_to_utf8(const vector<uint32_t> &codepoints) {
    string result;
    utf8::utf32to8(codepoints.begin(), codepoints.end(), back_inserter(result));
    return result;
}

#include <unordered_map>

unordered_map<uint32_t, uint32_t> createTarabarMap() {
    unordered_map<uint32_t, uint32_t> m;
    // Русские согласные
    m[0x411] = 0x429; // Б -> Щ
    m[0x429] = 0x411; // Щ -> Б
    m[0x412] = 0x428; // В -> Ш
    m[0x428] = 0x412; // Ш -> В
    m[0x413] = 0x427; // Г -> Ч
    m[0x427] = 0x413; // Ч -> Г
    m[0x414] = 0x426; // Д -> Ц
    m[0x426] = 0x414; // Ц -> Д
    m[0x416] = 0x425; // Ж -> Х
    m[0x425] = 0x416; // Х -> Ж
    m[0x417] = 0x424; // З -> Ф
    m[0x424] = 0x417; // Ф -> З
    m[0x41A] = 0x422; // К -> Т
    m[0x422] = 0x41A; // Т -> К
    m[0x41B] = 0x421; // Л -> С
    m[0x421] = 0x41B; // С -> Л
    m[0x41C] = 0x420; // М -> Р
    m[0x420] = 0x41C; // Р -> М
    m[0x41D] = 0x41F; // Н -> П
    m[0x41F] = 0x41D; // П -> Н
    // Строчные согласные
    m[0x431] = 0x449; // б -> щ
    m[0x449] = 0x431; // щ -> б
    m[0x432] = 0x448; // в -> ш
    m[0x448] = 0x432; // ш -> в
    m[0x433] = 0x447; // г -> ч
    m[0x447] = 0x433; // ч -> г
    m[0x434] = 0x446; // д -> ц
    m[0x446] = 0x434; // ц -> д
    m[0x436] = 0x445; // ж -> х
    m[0x445] = 0x436; // х -> ж
    m[0x437] = 0x444; // з -> ф
    m[0x444] = 0x437; // ф -> з
    m[0x43A] = 0x442; // к -> т
    m[0x442] = 0x43A; // т -> к
    m[0x43B] = 0x441; // л -> с
    m[0x441] = 0x43B; // с -> л
    m[0x43C] = 0x440; // м -> р
    m[0x440] = 0x43C; // р -> м
    m[0x43D] = 0x43F; // н -> п
    m[0x43F] = 0x43D; // п -> н
    // Русские гласные
    m[0x410] = 0x42F; // А -> Я
    m[0x42F] = 0x410; // Я -> А
    m[0x415] = 0x42E; // Е -> Ю
    m[0x42E] = 0x415; // Ю -> Е
    m[0x401] = 0x42D; // Ё -> Э
    m[0x42D] = 0x401; // Э -> Ё
    m[0x418] = 0x42B; // И -> Ы
    m[0x42B] = 0x418; // Ы -> И
    m[0x41E] = 0x423; // О -> У
    m[0x423] = 0x41E; // У -> О
    // Строчные гласные
    m[0x430] = 0x44F; // а -> я
    m[0x44F] = 0x430; // я -> а
    m[0x435] = 0x44E; // е -> ю
    m[0x44E] = 0x435; // ю -> е
    m[0x451] = 0x44D; // ё -> э
    m[0x44D] = 0x451; // э -> ё
    m[0x438] = 0x44B; // и -> ы
    m[0x44B] = 0x438; // ы -> и
    m[0x43E] = 0x443; // о -> у
    m[0x443] = 0x43E; // у -> о
    // Английские буквы
    m[0x41] = 0x5A; // A -> Z
    m[0x5A] = 0x41; // Z -> A
    m[0x42] = 0x59; // B -> Y
    m[0x59] = 0x42; // Y -> B
    m[0x43] = 0x58; // C -> X
    m[0x58] = 0x43; // X -> C
    m[0x44] = 0x57; // D -> W
    m[0x57] = 0x44; // W -> D
    m[0x45] = 0x56; // E -> V
    m[0x56] = 0x45; // V -> E
    m[0x46] = 0x55; // F -> U
    m[0x55] = 0x46; // U -> F
    m[0x47] = 0x54; // G -> T
    m[0x54] = 0x47; // T -> G
    m[0x48] = 0x53; // H -> S
    m[0x53] = 0x48; // S -> H
    m[0x49] = 0x52; // I -> R
    m[0x52] = 0x49; // R -> I
    m[0x4A] = 0x51; // J -> Q
    m[0x51] = 0x4A; // Q -> J
    m[0x4B] = 0x50; // K -> P
    m[0x50] = 0x4B; // P -> K
    m[0x4C] = 0x4F; // L -> O
    m[0x4F] = 0x4C; // O -> L
    m[0x4D] = 0x4E; // M -> N
    m[0x4E] = 0x4D; // N -> M
    // Строчные английские буквы
    m[0x61] = 0x7A; // a -> z
    m[0x7A] = 0x61; // z -> a
    m[0x62] = 0x79; // b -> y
    m[0x79] = 0x62; // y -> b
    m[0x63] = 0x78; // c -> x
    m[0x78] = 0x63; // x -> c
    m[0x64] = 0x77; // d -> w
    m[0x77] = 0x64; // w -> d
    m[0x65] = 0x76; // e -> v
    m[0x76] = 0x65; // v -> e
    m[0x66] = 0x75; // f -> u
    m[0x75] = 0x66; // u -> f
    m[0x67] = 0x74; // g -> t
    m[0x74] = 0x67; // t -> g
    m[0x68] = 0x73; // h -> s
    m[0x73] = 0x68; // s -> h
    m[0x69] = 0x72; // i -> r
    m[0x72] = 0x69; // r -> i
    m[0x6A] = 0x71; // j -> q
    m[0x71] = 0x6A; // q -> j
    m[0x6B] = 0x70; // k -> p
    m[0x70] = 0x6B; // p -> k
    m[0x6C] = 0x6F; // l -> o
    m[0x6F] = 0x6C; // o -> l
    m[0x6D] = 0x6E; // m -> n
    m[0x6E] = 0x6D; // n -> m
    return m;
}

string encrypt_tarabar(const string &input) {
    auto table = createTarabarMap();
    vector<uint32_t> cps = utf8_to_codepoints(input);
    vector<uint32_t> new_cps;

    for (auto &cp : cps) {
        if(cp == U' ') continue;
        auto it = table.find(cp);
        if (it != table.end()) cp = it->second;
        new_cps.push_back(cp);
    }
    return codepoints_to_utf8(new_cps);
}

string decrypt_tarabar(const string &input) {
    auto table = createTarabarMap();

    unordered_map<uint32_t, uint32_t> inverse_table;
    for (const auto& p : table) {
        inverse_table[p.second] = p.first;
    }
    vector<uint32_t> cps = utf8_to_codepoints(input);
    for (auto &cp : cps) {
        auto it = inverse_table.find(cp);
        if (it != inverse_table.end())
            cp = it->second;
    }
    return codepoints_to_utf8(cps);
}

bool process_file_tarabar(const string &input_filename, const string &output_filename, bool encrypt) {
    ifstream infile(input_filename, ios::binary);
    if (!infile) {
        cerr << "Не удалось открыть файл для чтения: " << input_filename << endl;
        return false;
    }

    string content((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    string result;
    try {
        result = encrypt ? encrypt_tarabar(content) : decrypt_tarabar(content);
    } catch (const std::exception &e) {
        cerr << "Ошибка при обработке UTF-8: " << e.what() << endl;
        return false;
    }

    ofstream outfile(output_filename, ios::binary);
    if (!outfile) {
        cerr << "Не удалось открыть файл для записи: " << output_filename << endl;
        return false;
    }

    outfile << result;
    outfile.close();

    cout << "Файл " << (encrypt ? "зашифрован" : "расшифрован")
         << " и сохранён как: " << output_filename << endl;
    return true;
}

void process_terminal_tarabar(bool encrypt) {
    cout << "Введите текст: ";
    string text;
    getline(cin, text);

    try {
        string result;
        if (encrypt) {
            result = encrypt_tarabar(text);
            cout << "Зашифрованный текст:\n" << result << endl;
        } else {
            result = decrypt_tarabar(text);
            cout << "Дешифрованный текст:\n" << result << endl;
        }
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}

void menu_tarabar() {
    while (true) {
        cout << "\nТарабарский шифр:\n"
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
        bool encrypt = (action == 1);

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
                string result = encrypt ? encrypt_tarabar(text) : decrypt_tarabar(text);
                cout << (encrypt ? "Зашифрованный" : "Дешифрованный") << " текст:\n" << result << endl;
            } else {
                cout << "Введите имя входного файла: ";
                string inFile;
                getline(cin, inFile);
                string pass = Password();
                cout << "Введите имя выходного файла: ";
                string outFile;
                getline(cin, outFile);
                ifstream infile(inFile, ios::binary);
                if (!infile) {
                    cerr << "Не удалось открыть файл для чтения: " << inFile << endl;
                    continue;
                }
                string text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
                infile.close();
                if (encrypt){
                    ofstream outfile(outFile + "_enc", ios::binary);
                    string result = encrypt_tarabar(text);
                    outfile << result;
                    if (!outfile) {
                        cerr << "Не удалось открыть файл для записи: " << outFile << endl;
                    continue;
                    }
                    outfile.close();
                } else{
                    ofstream outfile(outFile + "_dec", ios::binary);
                    string result = decrypt_tarabar(text);
                    outfile << result;
                    if (!outfile) {
                        cerr << "Не удалось открыть файл для записи: " << outFile << endl;
                    continue;
                    }
                    outfile.close(); 
                }
                cout << "Готово!\n";
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }
}


