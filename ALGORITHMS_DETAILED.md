# Подробный разбор трёх алгоритмов шифрования

Этот документ содержит детальное объяснение логики каждого алгоритма с примерами.

---

## АЛГОРИТМ 1: КОДОВОЕ СЛОВО (ВИЖЕНЕР) — codeword/codeword.cpp

### Исторический контекст
Шифр Виженера — это полиалфавитный шифр подстановки, созданный Жаном де Виженером в XVI веке. Он усовершенствовал шифр Цезаря, добавив **переменное смещение**.

### Основная идея
Вместо того, чтобы сдвигать **каждую букву на одинаковую величину** (как в Цезаре), в Виженере используется **таблица подстановки**, которая зависит от кодового слова.

### Пример с английским алфавитом

**Кодовое слово:** `HELLO`

**Шаг 1: Создание таблицы подстановки**

Стандартный алфавит: `a b c d e f g h i j k l m n o p q r s t u v w x y z`

Процесс в коде:
```cpp
vector<uint32_t> createSubstitutionTableEnglish(const string& codeWord) {
    vector<uint32_t> table;
    unordered_set<uint32_t> used;
    
    // Фаза 1: Добавляем уникальные буквы из кодового слова в порядке появления
    // "HELLO" → 'H', 'E', 'L', 'L', 'O'
    // Уникальные (в нижнем регистре): 'h', 'e', 'l', 'o'
    // table = ['h', 'e', 'l', 'o']
    
    // Фаза 2: Добавляем оставшиеся буквы алфавита
    // table = ['h', 'e', 'l', 'o', 'a', 'b', 'c', 'd', 'f', 'g', 'i', 'j', 'k', 'm', 'n', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']
    
    return table;
}
```

**Визуальная таблица подстановки:**
```
Позиция (индекс):  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
Исходный алфавит:  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
Таблица:           h  e  l  o  a  b  c  d  f  g  i  j  k  m  n  p  q  r  s  t  u  v  w  x  y  z
```

**Шаг 2: Шифрование текста**

Текст: `hello world`

```cpp
uint32_t encryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table) {
    // cp = кодовая точка символа (например, 'a' = 97)
    // table = таблица подстановки
    
    bool isUpper = (cp >= 0x41 && cp <= 0x5A);  // A-Z
    uint32_t lower = toLower(cp);               // Приводим к нижнему регистру
    int idx = (int)(lower - 0x61);              // Индекс в алфавите (0-25)
    
    // Пример: 'a' → idx = 0 → table[0] = 'h'
    // Пример: 'e' → idx = 4 → table[4] = 'a'
    
    uint32_t encrypted = table[idx];
    
    if (isUpper) {
        // Сохраняем регистр
        return (uint32_t)((char)(encrypted - 0x61 + 0x41));  // h → H
    }
    return encrypted;
}
```

**Шифрование символа за символом:**
```
h → позиция в алфавите = 7 → table[7] = 'd' ✓
e → позиция = 4 → table[4] = 'a' ✓
l → позиция = 11 → table[11] = 'j' ✓
l → позиция = 11 → table[11] = 'j' ✓
o → позиция = 14 → table[14] = 'n' ✓
  → пробел остаётся пробелом (не буква)
w → позиция = 22 → table[22] = 'w' ✓
o → позиция = 14 → table[14] = 'n' ✓
r → позиция = 17 → table[17] = 'r' ✓
l → позиция = 11 → table[11] = 'j' ✓
d → позиция = 3 → table[3] = 'o' ✓
```

**Результат шифрования:** `dajjn wnjjo`

**Шаг 3: Дешифрование**

```cpp
uint32_t decryptCharEnglishCp(uint32_t cp, const vector<uint32_t>& table) {
    // Обратное преобразование
    // Находим, какой исходный индекс привёл к этой букве в таблице
    
    for (int i = 0; i < (int)table.size(); ++i) {
        if (table[i] == lower) {  // Нашли букву в таблице
            uint32_t dec = (uint32_t)(0x61 + i);  // Восстанавливаем исходную букву
            return dec;
        }
    }
}
```

Например, при дешифровании `d`:
- Ищем `d` в table
- Находим на позиции 3
- Исходная буква = 'a' + 3 = 'd' ✓

### Поддержка русского языка (UTF-8)

**Русский алфавит (кириллица):**
- Строчные: а б в г д е ё ж з и й к л м н о п р с т у ф х ц ч ш щ ъ ы ь э ю я
- Заглавные: А Б В Г Д Е Ё Ж З И Й К Л М Н О П Р С Т У Ф Х Ц Ч Ш Щ Ъ Ы Ь Э Ю Я

**UTF-8 кодирование:**
```
а (U+0430) = 0xD0 0xB0 (2 байта)
А (U+0410) = 0xD0 0x90 (2 байта)
ё (U+0451) = 0xD1 0x91 (2 байта)
Ё (U+0401) = 0xD0 0x81 (2 байта)
```

**Преобразование UTF-8 в кодовую точку:**
```cpp
uint32_t utf8_to_cp(const string& s, size_t pos, size_t& bytes) {
    unsigned char c0 = s[pos];
    
    if (c0 < 0x80) {
        // ASCII (1 байт)
        bytes = 1;
        return (uint32_t)c0;  // Например, 'a' = 0x61
    }
    
    if (pos + 1 < s.size()) {
        unsigned char c1 = s[pos+1];
        // UTF-8 (2 байта для кириллицы)
        bytes = 2;
        uint32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        // Пример: 0xD0 0xB0 → 0x0430 (а)
        return cp;
    }
}
```

**Создание таблицы для русского:**
```cpp
vector<uint32_t> createSubstitutionTableCyrillic(const string& codeWord) {
    // Аналогично английскому, но:
    // - Декодируем UTF-8 в кодовые точки
    // - Используем функции cyrillicCpToIndex() и cyrillicIndexToCp()
    
    // Индекс ё = 6 (между 'е' и 'ж')
    // Остальные буквы маппируются по порядку
}
```

### Защита паролем

```cpp
extern "C" {
    void codewordEncrypt(const string& inputPath, const string& outputPath) {
        // 1. Запросить кодовое слово
        cout << "Введите кодовое слово (ASCII или русские буквы): ";
        string codeWord;
        getline(cin, codeWord);
        
        // 2. Проверить корректность (только уникальные буквы)
        if (!isValidCodeWord(codeWordBytes)) {
            cerr << "Ошибка: некорректное кодовое слово\n";
            return;
        }
        
        // 3. Запросить пароль для защиты файла
        string password = Password();  // Запрашивает и подтверждает
        
        // 4. Читать входной файл
        vector<unsigned char> fileBytes = readFileBinary(inputPath);
        
        // 5. Шифровать
        vector<unsigned char> encryptedBytes = encrypt(fileBytes, codeWordBytes);
        
        // 6. Записать в выходной файл
        writeFileBinary(outputPath, encryptedBytes);
        
        // 7. Сохранить хеш пароля в отдельный файл
        savePasswordToFile(outputPath, password);
        // Создаёт файл: outputPath + ".codeword"
        // Содержимое: хеш пароля
        
        cout << "Файл зашифрован!\n";
    }
    
    void codewordDecrypt(const string& inputPath, const string& outputPath) {
        // 1. Проверить пароль перед расшифровкой
        cout << "Введите пароль для файла: ";
        string password;
        getline(cin, password);
        
        if (!checkPasswordFromFile(inputPath, password)) {
            cerr << "Ошибка: неверный пароль!\n";
            return;  // Отклоняем, если пароль неверный
        }
        
        // 2. Запросить кодовое слово
        // 3. Прочитать входной файл
        // 4. Дешифровать
        // 5. Записать выходной файл
    }
}
```

### Проверка кодового слова

```cpp
bool isValidCodeWord(const vector<unsigned char>& codeWord) {
    if (codeWord.empty()) return false;
    
    unordered_set<string> seenChars;  // Отслеживаем видимые символы
    size_t i = 0;
    
    while (i < codeWord.size()) {
        unsigned char c = codeWord[i];
        
        // ASCII буква
        if ((c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A)) {
            string key(1, toLower(c));
            if (seenChars.count(key)) return false;  // Дублирование!
            seenChars.insert(key);
            i++;
        }
        // UTF-8 кириллица (2 байта)
        else if (i + 1 < codeWord.size() && 
                 ((c == 0xD0 && ((codeWord[i+1] >= 0x90 && codeWord[i+1] <= 0xBF) || codeWord[i+1] == 0x81)) ||
                  (c == 0xD1 && ((codeWord[i+1] >= 0x80 && codeWord[i+1] <= 0x8F) || codeWord[i+1] == 0x91)))) {
            string key(codeWord.begin() + i, codeWord.begin() + i + 2);
            if (key == "\xD0\x81") key = "\xD1\x91";  // Нормализуем Ё
            if (seenChars.count(key)) return false;   // Дублирование!
            seenChars.insert(key);
            i += 2;
        } else {
            return false;  // Недопустимый символ
        }
    }
    
    return !seenChars.empty();
}
```

**Требование**: Кодовое слово должно содержать **только буквы** (ASCII или кириллица) и **все буквы должны быть уникальными**.

Примеры:
- ✅ `HELLO` — валидно
- ✅ `hello` — валидно (регистр не важен)
- ✅ `привет` — валидно (русский)
- ❌ `HELLOO` — НЕВАЛИДНО (две L)
- ❌ `HELLO!` — НЕВАЛИДНО (восклицательный знак)

---

## АЛГОРИТМ 2: ТАБЛИЧНАЯ ПЕРЕСТАНОВКА — tablet/tablet.cpp

### Исторический контекст
Это один из древнейших методов шифрования, использовавшийся ещё в древние времена. Суть: **переставлять, а не заменять**.

### Основная идея
1. Разбить текст на блоки (8×8 = 64 байта)
2. Расположить каждый блок как матрицу
3. Переставить строки и столбцы согласно ключам
4. Собрать переставленные блоки обратно

### Пример работы

**Блок данных (8×8):**
```
Исходный блок:
A B C D E F G H
I J K L M N O P
Q R S T U V W X
Y Z 1 2 3 4 5 6
7 8 9 0 ! @ # $
% ^ & * ( ) - =
[ ] { } \ | ; :
' " < > , . / ?
```

**Ключи перестановки:**
- Ключ столбцов: `[3, 1, 4, 1, 5, 9, 2, 6]` — но это может быть любая перестановка 0-7
- Ключ строк: `[5, 2, 7, 1, 0, 3, 4, 6]` — перестановка индексов строк

**Шаг 1: Перестановка по столбцам**

```cpp
vector<unsigned char> Shuffle(const vector<unsigned char>& block, 
                             const vector<int>& col_key, 
                             const vector<int>& row_key) {
    size_t n = static_cast<size_t>(sqrt(block.size()));  // n = 8
    vector<unsigned char> temp = block;
    vector<unsigned char> new_block(block.size());
    
    // Для каждой строки переставляем столбцы согласно col_key
    for (size_t i = 0; i < n; ++i) {  // i = 0..7 (строки)
        for (size_t j = 0; j < n; ++j) {  // j = 0..7 (столбцы)
            // Берём элемент (i, j) и кладём его в позицию (i, col_key[j])
            if (i * n + j < block.size() && i * n + col_key[j] < block.size()) {
                temp[i * n + col_key[j]] = block[i * n + j];
            }
        }
    }
```

Визуально для 1-й строки с col_key = `[3, 1, 4, 1, 5, 9, 2, 6]`:
```
До:      A B C D E F G H
Индексы: 0 1 2 3 4 5 6 7

После перестановки столбцов:
Новая позиция каждого элемента:
- A (старая позиция 0) → новая позиция col_key[0] = 3
- B (старая позиция 1) → новая позиция col_key[1] = 1
- C (старая позиция 2) → новая позиция col_key[2] = 4
- ...

Результат: ? B ? A C ? G ? (на позициях 0,1,2,3,4,5,6,7)
```

**Шаг 2: Перестановка по строкам**

```cpp
    // Для каждого столбца переставляем строки согласно row_key
    for (size_t i = 0; i < n; ++i) {  // i = 0..7 (новые строки)
        for (size_t j = 0; j < n; ++j) {  // j = 0..7 (столбцы)
            // Берём элемент из строки row_key[i], столбца j
            // и кладём в новую строку i, столбец j
            if (row_key[i] * n + j < block.size() && i * n + j < block.size()) {
                new_block[row_key[i] * n + j] = temp[i * n + j];
            }
        }
    }
    
    return new_block;
}
```

С row_key = `[5, 2, 7, 1, 0, 3, 4, 6]`:
```
После перестановки строк строка i берётся из позиции row_key[i]:
- Новая строка 0 ← Старая строка 5
- Новая строка 1 ← Старая строка 2
- Новая строка 2 ← Старая строка 7
- ...
```

### Дешифрование

```cpp
vector<unsigned char> Unshuffle(const vector<unsigned char>& block, 
                               const vector<int>& col_key, 
                               const vector<int>& row_key) {
    // Обратная перестановка в ОБРАТНОМ порядке
    
    // Шаг 1: Обратная перестановка по строкам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (row_key[i] * n + j < block.size() && i * n + j < block.size()) {
                temp[i * n + j] = block[row_key[i] * n + j];
            }
        }
    }
    
    // Шаг 2: Обратная перестановка по столбцам
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i * n + col_key[j] < block.size() && i * n + j < block.size()) {
                new_block[i * n + j] = temp[i * n + col_key[j]];
            }
        }
    }
    
    return new_block;
}
```

**Важно:** Дешифрование **НЕ** просто применяет те же ключи. Оно **обращает операцию**, т.е. восстанавливает исходный порядок.

### Генерация ключей

```cpp
vector<int> generateRandomKey(size_t size) {
    vector<int> key(size);
    
    // Инициализируем ключ как [0, 1, 2, 3, 4, 5, 6, 7]
    for (size_t i = 0; i < size; ++i) {
        key[i] = i;
    }
    
    // Перемешиваем (алгоритм Фишера-Йетса)
    random_device rd;
    mt19937 g(rd());
    shuffle(key.begin(), key.end(), g);
    
    return key;  // Результат: случайная перестановка, например [5, 2, 7, 1, 0, 3, 4, 6]
}
```

Ключ генерируется при **шифровании** и сохраняется в файл `.keys`.

### Сохранение ключей

```cpp
extern "C" {
    void tabletEncrypt(const string& inputPath, const string& outputPath) {
        string password = Password();
        
        // Генерируем случайные ключи
        vector<int> col_key = generateRandomKey(TABLET_BLOCK_DIM);  // 8
        vector<int> row_key = generateRandomKey(TABLET_BLOCK_DIM);
        
        cout << "Ключ столбцов: ";
        for (int k : col_key) cout << k << " ";  // 5 2 7 1 0 3 4 6
        cout << endl;
        
        // Сохраняем ключи и пароль в отдельный файл
        string keyFile = outputPath + ".keys";
        ofstream out(keyFile, ios::binary);
        
        // 1. Записываем хеш пароля
        string passHash = hashPassword(password);
        size_t passLen = passHash.length();
        out.write((char*)&passLen, sizeof(passLen));
        out.write(passHash.c_str(), passLen);
        
        // 2. Записываем размерность (8)
        unsigned char dim = TABLET_BLOCK_DIM;
        out.write((char*)&dim, sizeof(dim));
        
        // 3. Записываем ключи столбцов (8 байт)
        for (int k : col_key) {
            unsigned char byte = k;
            out.write((char*)&byte, sizeof(byte));
        }
        
        // 4. Записываем ключи строк (8 байт)
        for (int k : row_key) {
            unsigned char byte = k;
            out.write((char*)&byte, sizeof(byte));
        }
        out.close();
        
        // Шифруем файл блоками
        vector<unsigned char> fileBytes = readFileBinary(inputPath);
        vector<unsigned char> encryptedBytes = Encrypt(fileBytes, TABLET_BLOCK_SIZE, col_key, row_key);
        writeFileBinary(outputPath, encryptedBytes);
    }
}
```

**Структура файла `.keys`:**
```
[пароль_длина (8 байт)][пароль_хеш (X байт)][размерность (1 байт)][col_key (8 байт)][row_key (8 байт)]
```

### Работа с блоками

```cpp
vector<unsigned char> Encrypt(const vector<unsigned char>& text, 
                             size_t block_size, 
                             const vector<int>& col_key, 
                             const vector<int>& row_key) {
    vector<unsigned char> result;
    
    // Обрабатываем текст блоками по 64 байта
    for (size_t start = 0; start < text.size(); start += block_size) {
        // Извлекаем блок (если последний блок меньше 64 байт, он дополняется нулями)
        vector<unsigned char> block = MakeBlock(text, start, block_size);
        
        // Переставляем блок
        vector<unsigned char> shuffled = Shuffle(block, col_key, row_key);
        
        // Добавляем к результату
        result.insert(result.end(), shuffled.begin(), shuffled.end());
    }
    return result;
}
```

**Пример:**
- Входной файл: 200 байт
- Блоки: 200 / 64 = 3 блока (64 + 64 + 72, но последний дополняется до 64)
- Каждый блок отдельно переставляется
- Результат: 192 байта (3 × 64)

---

## АЛГОРИТМ 3: ТАРАБАРСКАЯ ГРАМОТА — tarabar/tar.cpp

### Исторический контекст
Это русский криптографический метод, описанный в XIII веке. Суть: **зеркальная замена букв в алфавите**.

### Основная идея
Каждая буква заменяется на свою "пару" в алфавите:
- Первая и последняя
- Вторая и предпоследняя
- И так далее...

### Таблица замен для русского

```cpp
static void buildTarabarMaps(unordered_map<uint32_t,uint32_t>& enc, 
                             unordered_map<uint32_t,uint32_t>& dec) {
    // Гласные: зеркалируются через "средину"
    const vector<pair<uint32_t,uint32_t>> rusVowels = {
        {0x0430, 0x044F}, // а (U+0430) ↔ я (U+044F)
        {0x0435, 0x044E}, // е (U+0435) ↔ ю (U+044E)
        {0x0451, 0x044D}, // ё (U+0451) ↔ э (U+044D)
        {0x0438, 0x044B}, // и (U+0438) ↔ ы (U+044B)
        {0x043E, 0x0443}  // о (U+043E) ↔ у (U+0443)
    };
    for (auto &p : rusVowels) {
        uint32_t a = p.first, b = p.second;
        enc[a] = b;     // а → я
        dec[b] = a;     // я → а
        dec[a] = b;     // а → я (для дешифрования)
        enc[b] = a;     // я → а
        
        // Также заглавные версии
        uint32_t A = (a==0x0451?0x0401:(a - 0x20));
        uint32_t B = (b==0x0451?0x0401:(b - 0x20));
        enc[A] = B;
        dec[B] = A;
    }
    
    // Согласные: в пары, зеркально
    const vector<pair<uint32_t,uint32_t>> rusConsonantPairs = {
        {0x0431, 0x0449}, // б ↔ щ
        {0x0432, 0x0448}, // в ↔ ш
        {0x0433, 0x0447}, // г ↔ ч
        {0x0434, 0x0446}, // д ↔ ц
        {0x0436, 0x0445}, // ж ↔ х
        {0x0437, 0x0444}, // з ↔ ф
        {0x043A, 0x0442}, // к ↔ т
        {0x043B, 0x0441}, // л ↔ с
        {0x043C, 0x0440}, // м ↔ р
        {0x043D, 0x043F}  // н ↔ п
    };
    for (auto &p : rusConsonantPairs) {
        uint32_t l = p.first, r = p.second;
        enc[l] = r;  // Левый → Правый
        enc[r] = l;  // Правый → Левый (потому что это симметрично)
        dec[l] = r;  // То же самое для дешифрования
        dec[r] = l;
        // Заглавные версии
        // ...
    }
}
```

### Пример шифрования

**Текст:** `привет`

```
п (0x043F) ↔ н (0x043D) → н
р (0x0440) ↔ м (0x043C) → м
и (0x0438) ↔ ы (0x044B) → ы
в (0x0432) ↔ ш (0x0448) → ш
е (0x0435) ↔ ю (0x044E) → ю
т (0x0442) ↔ к (0x043A) → к

Результат: нмышюк
```

**При дешифровании:**
```
н ↔ п → п
м ↔ р → р
ы ↔ и → и
ш ↔ в → в
ю ↔ е → е
к ↔ т → т

Результат: привет ✓
```

### Главное свойство: РЕВЕРСИВНОСТЬ

Таблица **симметричная**:
```cpp
enc[a] = b;
enc[b] = a;  // Обратная замена тоже есть в таблице
```

Поэтому применяя функцию дважды, получим исходный текст:
```
Текст: "привет"
Шифрование: "нмышюк"
Шифрование ещё раз: "привет"
```

Это означает, что для шифрования и дешифрования можно использовать **одну и ту же таблицу**.

### Работа с UTF-8

```cpp
vector<unsigned char> encrypt_tarabar(const vector<unsigned char>& input) {
    unordered_map<uint32_t,uint32_t> enc, dec;
    buildTarabarMaps(enc, dec);
    
    vector<unsigned char> result;
    size_t i = 0;
    
    while (i < input.size()) {
        size_t bytes = 0;
        uint32_t cp = utf8_to_cp(input, i, bytes);  // Декодируем UTF-8
        
        uint32_t outcp = cp;
        auto it = enc.find(cp);
        if (it != enc.end()) {
            outcp = it->second;  // Заменяем букву
        }
        
        cp_to_utf8(outcp, result);  // Кодируем обратно в UTF-8
        i += (bytes > 0 ? bytes : 1);
    }
    
    return result;
}
```

**Процесс:**
1. Читаем кодовую точку (может быть 1-3 байта)
2. Смотрим в таблице, есть ли замена
3. Если нет — оставляем как есть
4. Кодируем обратно в UTF-8

### Сохранение пароля

```cpp
extern "C" {
    void tarabarEncrypt(const string& inputPath, const string& outputPath) {
        string password = Password();
        
        vector<unsigned char> fileBytes = readFileBinary(inputPath);
        vector<unsigned char> encryptedBytes = encrypt_tarabar(fileBytes);
        
        writeFileBinary(outputPath, encryptedBytes);
        
        // Сохраняем хеш пароля
        string metaFile = outputPath + ".tarabar";
        ofstream meta(metaFile);
        meta << hashPassword(password);
        meta.close();
        
        cout << "✓ Файл зашифрован!\n";
    }
}
```

**Файл `.tarabar` содержит:**
- Одна строка: хеш пароля

### Таблица для английского

```cpp
const vector<pair<uint32_t,uint32_t>> engVowels = {
    {0x61, 0x75}, // a ↔ u
    {0x65, 0x6F}, // e ↔ o
    {0x69, 0x69}, // i ↔ i (сам на себя)
    {0x6F, 0x65}, // o ↔ e
    {0x75, 0x61}  // u ↔ a
};

const vector<pair<uint32_t,uint32_t>> engCons = {
    {0x62, 0x6E}, // b ↔ n
    {0x63, 0x70}, // c ↔ p
    {0x64, 0x71}, // d ↔ q
    // ... и т.д.
    {0x6D, 0x7A}  // m ↔ z
};
```

**Пример:** `hello` → `iallo` (при наличии всех замен в таблице)

---

## СРАВНЕНИЕ ТРЁХ АЛГОРИТМОВ

| Характеристика | Виженер | Табличная перест. | Тарабарская |
|---|---|---|---|
| **Тип** | Подстановка | Перестановка | Замена |
| **Ключ** | Кодовое слово | Два вектора (8+8) | Таблица замен (фиксированная) |
| **Размер блока** | 1 символ | 64 байта | 1 символ |
| **Реверсивность** | Нет (нужна таблица) | Нет (нужна обратная операция) | Да (применить дважды) |
| **Безопасность** | Средняя | Средняя | Слабая (фиксированная таблица) |
| **Древность** | XVI век | Древние времена | XIII век |
| **Сложность кода** | Средняя | Средняя | Простая |

---

## ПОЧЕМУ ЭТИ АЛГОРИТМЫ НЕ ИСПОЛЬЗУЮТ В РЕАЛЬНОСТИ

1. **Виженер** — уязвим к криптоанализу частоты (известно, что буквы встречаются с разной частотой)
2. **Табличная перестановка** — можно раскрыть перебором ключей (8! = 40,320 вариантов на ключ)
3. **Тарабарская** — фиксированная таблица, легко раскрыть анализом частот

Для реальной защиты используют:
- AES, RSA, ECDSA (асимметричные)
- Хеширование: SHA-256, bcrypt

Эти алгоритмы используются **только для образовательных целей**.
