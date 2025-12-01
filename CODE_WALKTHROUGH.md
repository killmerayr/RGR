# Полный разбор кода: Программа шифрования файлов

Этот документ содержит детальный разбор каждого .cpp файла с объяснением логики и кода.

---

## ФАЙЛ 1: main/main.cpp — ГЛАВНАЯ ПРОГРАММА

### Что это делает
Это **точка входа** в программу. Она:
1. Инициализирует локаль для русского языка
2. Показывает интерактивное меню
3. Динамически загружает библиотеки шифров
4. Вызывает нужный алгоритм
5. Обрабатывает ошибки

### Построчный разбор

#### Заголовки
```cpp
#include <iostream>
#include <stdexcept>
#include <string>
#include <limits>
#include <dlfcn.h>           // Для dlopen/dlsym - динамическая загрузка библиотек
#include <clocale>           // Для setlocale - поддержка русского языка
#include "menuUtils.hpp"
```

**Ключевой момент**: `<dlfcn.h>` — это UNIX-specific заголовок для работы с динамическими библиотеками.

#### Определение типа функции
```cpp
typedef void (*CryptoFunc)(const string&, const string&);
```

Это **указатель на функцию**, которая принимает два string и ничего не возвращает:
- `inputPath` — путь к исходному файлу
- `outputPath` — путь к выходному файлу

#### Главная функция processLibrary
```cpp
bool processLibrary(const string& libPath,           // путь к .so файлу
                    const string& encFunc,           // имя функции шифрования
                    const string& decFunc,           // имя функции дешифрования
                    CryptoMode action,               // что делать: шифровать или дешифровать
                    const string& filePath,          // путь к входному файлу
                    const string& cryptoFilePath)    // путь к выходному файлу
```

**Шаг 1: Загрузка библиотеки**
```cpp
void* handler = dlopen(libPath.c_str(), RTLD_LAZY);
if (!handler) {
    cerr << "Ошибка: " << dlerror() << endl;  // dlerror() выводит понятную ошибку
    return false;
}
```

- `dlopen(path, flags)` — загружает динамическую библиотеку
- `RTLD_LAZY` — ленивое связывание (функции ищутся при вызове, не при загрузке)
- `handler` — уникальный идентификатор загруженной библиотеки
- `dlerror()` — возвращает строку ошибки, если что-то не так

**Шаг 2: Получение функции из библиотеки**
```cpp
typedef void (*CryptoFunc)(const string&, const string&);

if (action == CryptoMode::Encryption) {
    CryptoFunc encrypt = (CryptoFunc) dlsym(handler, encFunc.c_str());
    if (!encrypt) {
        cerr << "Функция не найдена: " << encFunc << endl;
        dlclose(handler);
        return false;
    }
    (*encrypt)(filePath, cryptoFilePath);  // Вызываем функцию
}
```

- `dlsym(handler, "functionName")` — получает адрес функции из загруженной библиотеки
- `(CryptoFunc)` — приводит адрес к типу функции
- `(*encrypt)(...)` — вызывает функцию через указатель

**Шаг 3: Закрытие библиотеки**
```cpp
dlclose(handler);  // Освобождаем ресурсы
```

---

#### Функция main()

```cpp
int main() {
    setlocale(LC_ALL, "");  // Включаем русский язык для консоли
    ios::sync_with_stdio(false);  // Оптимизация ввода-вывода
    cin.tie(nullptr);             // Отвязываем cin от cout
    
    clearScreen();
    
    while (true) {  // Бесконечный цикл — программа выходит только на 0
```

**Основной цикл программы:**

```
1️⃣ showMenu(StartMenu)
   ├─ 1: MatrixCipher (Табличная перестановка)
   ├─ 2: VigenereCipher (Кодовое слово)
   ├─ 3: TarabarCipher (Тарабарская грамота)
   └─ 0: ExitProgram (Выход)

2️⃣ getCryptoAlgorithm()
   └─ Пользователь выбирает алгоритм

3️⃣ Если выход → break и return 0

4️⃣ showMenu(EncDecMenu)
   ├─ 1: Encryption (Шифрование)
   └─ 2: Decryption (Дешифрование)

5️⃣ getCryptoMod()
   └─ Пользователь выбирает операцию

6️⃣ getFilePath()
   └─ Запросить путь и проверить, существует ли файл

7️⃣ createModFile()
   └─ Создать имя выходного файла (добавить _encrypted или _decrypted)

8️⃣ processLibrary()
   └─ Загрузить нужную .so библиотеку и вызвать функцию

9️⃣ Повторить с шага 1️⃣
```

**Пример выбора алгоритма:**
```cpp
switch (userAlgorithm) {
    case Algorithm::MatrixCipher:
        success = processLibrary(
            "./lib/libTabletCipher.so",      // какую библиотеку грузим
            "tabletEncrypt",                  // функция для шифрования
            "tabletDecrypt",                  // функция для дешифрования
            action, 
            filePath, 
            cryptoFilePath
        );
        break;
```

Каждый алгоритм соответствует определённой `.so` библиотеке и двум функциям (encrypt/decrypt).

---

## ФАЙЛ 2: main/menuUtils.cpp — УТИЛИТЫ МЕНЮ

Это файл для работы с интерфейсом и вводом-выводом.

### clearScreen()
```cpp
void clearScreen() {
#ifdef _WIN32
    system("cls");      // Windows
#else
    system("clear");    // Linux/macOS
#endif
}
```

Кроссплатформенная очистка экрана. Удаляет все предыдущие сообщения для чистого интерфейса.

### showMenu()
```cpp
void showMenu(MenuMode mode) {
    clearScreen();
    if (mode == MenuMode::StartMenu) {
        cout << "¶════════════════════════════════════════¶\n";
        cout << "¶      Программа шифрования файлов       ¶\n";
        cout << "¶════════════════════════════════════════¶\n";
        cout << "¶  1. Табличная перестановка             ¶\n";
        cout << "¶  2. Кодовое слово (Виженер)            ¶\n";
        cout << "¶  3. Тарабарская грамота                ¶\n";
        cout << "¶  0. Выход                              ¶\n";
        cout << "¶════════════════════════════════════════¶\n";
```

Выводит красиво оформленное меню. Если это **StartMenu** — показывает выбор алгоритма. Если **EncDecMenu** — показывает выбор операции (шифровать/дешифровать).

### getCryptoAlgorithm()
```cpp
Algorithm getCryptoAlgorithm() {
    int choice;
    cin >> choice;       // Читаем цифру
    cin.ignore();        // Игнорируем символ новой строки
    
    switch (choice) {
        case 1: return Algorithm::MatrixCipher;
        case 2: return Algorithm::VigenereCipher;
        case 3: return Algorithm::TarabarCipher;
        case 0: return Algorithm::ExitProgram;
        default:
            cerr << "Некорректный выбор!\n";
            return Algorithm::ExitProgram;
    }
}
```

Преобразует числовой ввод в enum Algorithm. Важно `cin.ignore()` — без этого переводы строк остаются в буфере и портят следующий ввод.

### getFilePath()
```cpp
string getFilePath() {
    string filePath;
    cout << "\nВведите путь к файлу: ";
    getline(cin, filePath);  // Читаем всю строку (включая пробелы)
    
    if (!fs::exists(filePath)) {  // fs::exists - функция из <filesystem>
        cerr << "Файл не найден: " << filePath << endl;
        return "";
    }
    
    return filePath;
}
```

Запрашивает путь и **проверяет, существует ли файл**. Если нет — показывает ошибку и возвращает пустую строку. Используется `getline()`, а не `cin >>`, чтобы файл мог содержать пробелы.

### createModFile()
```cpp
string createModFile(const string& filePath, 
                    const string& postscript,    // "_encrypted" или "_decrypted"
                    CryptoMode action) {
    fs::path p(filePath);           // Парсим путь
    string filename = p.stem().string();     // Имя файла БЕЗ расширения
    string extension = p.extension().string();  // Расширение (.txt, .pdf и т.д.)
    string parent = p.parent_path().string(); // Папка, в которой находится файл
    
    if (parent.empty()) {
        parent = ".";  // Если файл в текущей папке
    }
    
    // Собираем новое имя: /path/filename_encrypted.txt
    string cryptoFileName = parent + "/" + filename + postscript + extension;
    return cryptoFileName;
}
```

**Пример:**
- Входной путь: `/Users/ray/Documents/secret.txt`
- postscript: `_encrypted`
- Результат: `/Users/ray/Documents/secret_encrypted.txt`

**Что здесь происходит:**
1. Разбиваем путь на компоненты
2. Берём имя файла `secret`
3. Берём расширение `.txt`
4. Добавляем postscript в середину

---

## ФАЙЛ 3: main/utils.cpp — УТИЛИТЫ ФАЙЛОВОЙ СИСТЕМЫ И ПАРОЛЕЙ

Это вспомогательные функции, используемые всеми алгоритмами.

### readFileBinary()
```cpp
std::vector<unsigned char> readFileBinary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для чтения: " + filename);
    }
    
    // Способ 1: Прыгаем в конец файла и читаем размер
    file.seekg(0, std::ios::end);           // Позиция = конец файла
    std::streamsize length = file.tellg();  // Узнаём размер
    
    // Способ 2: Возвращаемся в начало
    file.seekg(0, std::ios::beg);
    
    // Способ 3: Выделяем память и читаем
    std::vector<unsigned char> buffer;
    buffer.resize(length);
    file.read(reinterpret_cast<char*>(buffer.data()), length);
    
    return buffer;
}
```

**Шаг за шагом:**
1. Открываем файл в **бинарном режиме** (`std::ios::binary`)
   - Это важно! В текстовом режиме некоторые байты могут быть интерпретированы как управляющие символы
2. Прыгаем в **конец файла** и узнаём размер
3. Возвращаемся в **начало**
4. Читаем **весь файл целиком** в вектор

Используется `unsigned char` — это просто байты, без интерпретации.

### writeFileBinary()
```cpp
void writeFileBinary(const std::string& filename, const std::vector<unsigned char>& buffer) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}
```

Просто пишет вектор байтов в файл в **бинарном режиме**.

### Password()
```cpp
std::string Password() {
    std::string password, confirm;
    std::cout << "Введите пароль для файла: ";
    std::getline(std::cin, password);      // Первый ввод
    std::cout << "Подтвердите пароль: ";
    std::getline(std::cin, confirm);       // Второй ввод
    
    if (password != confirm) {
        std::cerr << "Пароли не совпадают! Повторите ввод\n";
        return Password();                  // Рекурсивный вызов если не совпадают
    }
    return password;
}
```

**Важно**: Пароль запрашивается **два раза** для проверки. Если не совпадают — функция вызывает саму себя (рекурсия) и спрашивает заново.

### hashPassword() и verifyPassword()
```cpp
std::string hashPassword(const std::string& password) {
    unsigned char hash[4];  // 4 байта для хеша
    std::memset(hash, 0, sizeof(hash));  // Обнуляем
    
    // XOR все символы пароля
    for (size_t i = 0; i < password.length(); ++i) {
        hash[i % 4] ^= password[i];  // XOR с позициями 0, 1, 2, 3, 0, 1, 2, 3...
    }
    
    // Конвертируем в hex строку (12 символов: 0xABCDEF...)
    char hexStr[16];
    std::snprintf(hexStr, sizeof(hexStr), "%02x%02x%02x%02x", 
                  hash[0], hash[1], hash[2], hash[3]);
    return std::string(hexStr);
}

bool verifyPassword(const std::string& inputPassword, const std::string& storedHash) {
    return hashPassword(inputPassword) == storedHash;
}
```

**Как это работает:**
1. Берём пароль и XOR-им его символы в 4 байта
2. Конвертируем в hex строку (например: `a1b2c3d4`)
3. При проверке хешируем введённый пароль и сравниваем с сохранённым

**Важно**: Это **ПРОСТОЕ** хеширование для демонстрации! В реальных приложениях используют bcrypt или Argon2.

### isLetter() — Поддержка UTF-8

```cpp
bool isLetter(const std::string& text, size_t pos, size_t& charLen) {
    unsigned char c = text[pos];
    
    // ASCII буква (1 байт)
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        charLen = 1;
        return true;
    }
    
    // UTF-8 кириллица (2 байта)
    if (pos + 1 < text.length()) {
        unsigned char c1 = text[pos];
        unsigned char c2 = text[pos + 1];
        
        // Проверяем, является ли пара кириллицей
        if ((c1 == 0xD0 && c2 >= 0x90) || (c1 == 0xD1 && c2 <= 0x8F) || 
            (c1 == 0xD0 && c2 == 0xB5) || (c1 == 0xD1 && c2 == 0x91)) {
            charLen = 2;
            return true;
        }
    }
    
    charLen = 1;
    return false;
}
```

**UTF-8 кодирование русских букв:**
- А (большое) = `0xD0 0x90`
- а (маленькое) = `0xD0 0xB0`
- я = `0xD1 0x8F`
- ё = `0xD1 0x91`

Функция проверяет, является ли текущая позиция буквой и возвращает её длину в байтах (1 для ASCII, 2 для кириллицы).

---

## ФАЙЛ 4: main/menuUtils.hpp и main/utils.h — ЗАГОЛОВКИ

### menuUtils.hpp
```cpp
enum class MenuMode {
    StartMenu,       // Меню выбора алгоритма
    EncDecMenu       // Меню выбора операции
};

enum class Algorithm {
    MatrixCipher,        // 1
    VigenereCipher,      // 2
    TarabarCipher,       // 3
    ExitProgram          // 0
};

enum class CryptoMode {
    Encryption,     // Шифрование
    Decryption      // Дешифрование
};
```

Это **перечисления (enum class)** — типобезопасный способ определить набор констант. Вместо магических чисел используем понятные имена.

### utils.h
Объявления функций для работы с файлами и паролями. Это **interface** — сама реализация в utils.cpp.

---

## ПОТОК ВЫПОЛНЕНИЯ: ПРИМЕР РАБОТЫ ПРОГРАММЫ

Представим, пользователь хочет зашифровать файл алгоритмом Виженера:

```
1. Пользователь запускает программу
   ./rgz

2. main() инициализирует локаль и входит в цикл while(true)

3. Показывается StartMenu
   ├─ 1. Табличная перестановка
   ├─ 2. Кодовое слово (Виженер)
   ├─ 3. Тарабарская грамота
   └─ 0. Выход

4. Пользователь вводит 2
   getCryptoAlgorithm() возвращает Algorithm::VigenereCipher

5. Показывается EncDecMenu
   ├─ 1. Шифрование
   └─ 2. Дешифрование

6. Пользователь вводит 1
   getCryptoMod() возвращает CryptoMode::Encryption

7. getFilePath() запрашивает путь
   Пользователь вводит: /Users/ray/secret.txt
   Проверяется, существует ли файл ✓

8. createModFile() создаёт имя выходного файла
   Результат: /Users/ray/secret_encrypted.txt

9. switch statement выбирает VigenereCipher
   ↓
   processLibrary() вызывается с параметрами:
   - libPath = "./lib/libCodewordCipher.so"
   - encFunc = "codewordEncrypt"
   - decFunc = "codewordDecrypt"
   - action = Encryption
   - filePath = "/Users/ray/secret.txt"
   - cryptoFilePath = "/Users/ray/secret_encrypted.txt"

10. Внутри processLibrary():
    ├─ dlopen("./lib/libCodewordCipher.so", RTLD_LAZY)
    │  ✓ Библиотека загружена
    ├─ dlsym(handler, "codewordEncrypt")
    │  ✓ Функция найдена
    ├─ (*encrypt)("/Users/ray/secret.txt", "/Users/ray/secret_encrypted.txt")
    │  → Функция из libCodewordCipher.so выполняется:
    │    1. Запрашивает кодовое слово
    │    2. Запрашивает пароль (два раза)
    │    3. Читает файл в вектор байтов
    │    4. Шифрует данные
    │    5. Записывает результат в новый файл
    │    6. Сохраняет хеш пароля в secret_encrypted.txt.codeword
    └─ dlclose(handler)

11. Программа выводит "✓ Файл зашифрован!"

12. Показывается линия разделения
    Цикл повторяется — снова показывается StartMenu

13. Если пользователь введёт 0 → выход из программы
```

---

## КЛЮЧЕВЫЕ КОНЦЕПЦИИ

### 1. Динамическая загрузка (dlopen/dlsym)
```cpp
// Вместо статического линкования:
//   gcc main.o codeword.o tablet.o tarabar.o -o rgz

// Используем динамическую загрузку:
void* handler = dlopen("./lib/libCodewordCipher.so");
CryptoFunc func = (CryptoFunc) dlsym(handler, "functionName");
```

**Преимущества:**
- Можно обновить библиотеку без перекомпиляции программы
- Можно выбрать, какие библиотеки загружать (не все сразу)
- Меньше требований при распространении

### 2. Типизированные указатели на функции
```cpp
typedef void (*CryptoFunc)(const string&, const string&);
```

Это типизированный указатель — гарантирует, что функция имеет правильную сигнатуру.

### 3. Бинарная работа с файлами
```cpp
std::vector<unsigned char> readFileBinary(filename);
```

**Почему `unsigned char`?**
- Это просто байты от 0 до 255
- Без интерпретации (в отличие от `char` или `string`)
- Работает с любыми файлами (текст, изображения, архивы и т.д.)

### 4. UTF-8 кодирование
```cpp
// ASCII: 1 байт (A = 0x41)
// UTF-8: 2+ байта
// Кириллица: 2 байта (А = 0xD0 0x90)
```

Программа поддерживает русский язык, декодируя UTF-8 строки в кодовые точки Unicode.

### 5. Безопасность пароля
```cpp
// Пароль запрашивается два раза
if (password != confirm) {
    return Password();  // Рекурсия до совпадения
}

// Сохраняется только хеш
savePasswordToFile(outputPath, hashPassword(password));
```

---

## ДИАГРАММА ВЗАИМОДЕЙСТВИЯ КОМПОНЕНТОВ

```
┌─────────────────────┐
│    main/main.cpp    │  ← Точка входа
│  (dlopen/dlsym)     │
└──────────┬──────────┘
           │
     ┌─────┴──────────────────────────────┐
     │                                     │
     ▼                                     ▼
┌─────────────────────┐    ┌──────────────────────┐
│  menuUtils.cpp      │    │    utils.cpp         │
│  - showMenu()       │    │  - readFileBinary()  │
│  - getFilePath()    │    │  - writeFileBinary() │
│  - createModFile()  │    │  - Password()        │
└─────────────────────┘    │  - hashPassword()    │
                            └──────────────────────┘
                                     │
                                     ▼
                            ┌──────────────────────┐
                            │   .so библиотеки     │
                            ├──────────────────────┤
                            │ libCodewordCipher.so │
                            │  libTabletCipher.so  │
                            │  libTarabarCipher.so │
                            └──────────────────────┘
```

---

## ЧТО НУЖНО ПОМНИТЬ ДЛЯ РАССКАЗА

1. **main.cpp — это оркестратор**
   - Не сам шифрует, а вызывает нужный алгоритм через dlopen/dlsym
   - Показывает меню и обрабатывает ввод

2. **menuUtils.cpp — красивый интерфейс**
   - Меню, очистка экрана, запрос пути
   - Парсинг пути и создание новых имён файлов

3. **utils.cpp — общие утилиты**
   - Чтение/запись файлов в бинарном режиме
   - Работа с паролями и их хешированием
   - Поддержка UTF-8 (русского языка)

4. **Enum классы — типобезопасность**
   - Вместо магических чисел (1, 2, 3, 0) используем Algorithm::VigenereCipher и т.д.
   - Ошибки перепутать типы будут выловлены на этапе компиляции

5. **Динамическая загрузка — главная архитектурная фишка**
   - dlopen загружает библиотеку в памяти
   - dlsym получает функцию из библиотеки
   - dlclose освобождает ресурсы
