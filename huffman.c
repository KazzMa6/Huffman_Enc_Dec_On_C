#include <stdio.h>  // Подключение стандартной библиотеки ввода-вывода
#include <stdlib.h>  // Подключение стандартной библиотеки для работы с памятью
#include <string.h>  // Подключение стандартной библиотеки для работы со строками

#define SYMBOL_COUNT 256  // Максимальное количество символов
#define MAX_CODE_LEN 256  // Максимальная длина кода

// Структура для представления узла дерева Хаффмана
typedef struct HuffNode {
    unsigned char value;  // Значение символа
    unsigned long long count;  // Частота символа
    struct HuffNode *left, *right;  // Указатели на левого и правого потомков
} HuffNode;

// Структура для представления минимальной кучи
typedef struct {
    HuffNode* array[SYMBOL_COUNT];  // Массив указателей на узлы дерева Хаффмана
    int count;  // Количество элементов в куче
} Heap;

// Структура для представления битового кода
typedef struct {
    char bits[MAX_CODE_LEN];  // Битовый код
    int length;  // Длина битового кода
} BitCode;

// Функция для вставки узла в кучу
void heap_insert(Heap* h, HuffNode* node) {
    int idx = h->count++;  // Увеличение счетчика элементов и сохранение индекса нового элемента
    while (idx > 0) {  // Поднятие узла вверх по куче
        int parent = (idx - 1) / 2;  // Индекс родителя
        if (h->array[parent]->count <= node->count) break;  // Если порядок верный, прерываем цикл
        h->array[idx] = h->array[parent];  // Перемещение родителя вниз
        idx = parent;  // Переход к родителю
    }
    h->array[idx] = node;  // Вставка нового узла
}

// Функция для удаления минимального элемента из кучи
HuffNode* heap_remove_min(Heap* h) {
    HuffNode* min = h->array[0];  // Сохранение минимального элемента
    HuffNode* last = h->array[--h->count];  // Удаление последнего элемента и сохранение его
    int idx = 0;  // Начинаем с корня
    while (1) {  // Опускание узла вниз по куче
        int l = 2 * idx + 1, r = 2 * idx + 2, smallest = idx;  // Индексы левого и правого потомков
        if (l < h->count && h->array[l]->count < h->array[smallest]->count) smallest = l;  // Находим наименьший элемент
        if (r < h->count && h->array[r]->count < h->array[smallest]->count) smallest = r;  // Находим наименьший элемент
        if (smallest == idx) break;  // Если порядок верный, прерываем цикл
        h->array[idx] = h->array[smallest];  // Перемещение наименьшего элемента вверх
        idx = smallest;  // Переход к наименьшему потомку
    }
    h->array[idx] = last;  // Вставка последнего элемента
    return min;  // Возврат минимального элемента
}

// Функция для присвоения кодов символам
void assign_codes(HuffNode* node, BitCode* codes, char* temp, int len) {
    if (!node->left && !node->right) {  // Если узел является листом
        temp[len] = '\0';  // Завершение строки
        strcpy(codes[node->value].bits, temp);  // Копирование битового кода
        codes[node->value].length = len;  // Установка длины битового кода
        return;
    }
    if (node->left) {  // Если есть левый потомок
        temp[len] = '0';  // Добавляем '0' к коду
        assign_codes(node->left, codes, temp, len + 1);  // Рекурсивное присвоение кодов левому поддереву
    }
    if (node->right) {  // Если есть правый потомок
        temp[len] = '1';  // Добавляем '1' к коду
        assign_codes(node->right, codes, temp, len + 1);  // Рекурсивное присвоение кодов правому поддереву
    }
}

// Функция для освобождения памяти дерева Хаффмана
void free_tree(HuffNode* root) {
    if (!root) return;  // Если узел не существует, возвращаемся
    free_tree(root->left);  // Рекурсивное освобождение памяти левого поддерева
    free_tree(root->right);  // Рекурсивное освобождение памяти правого поддерева
    free(root);  // Освобождение памяти текущего узла
}

int main() {
    FILE *fin = fopen("input.txt", "rb");  // Открытие файла для чтения
    if (!fin) {  // Если файл не открыт
        perror("input.txt");  // Вывод сообщения об ошибке
        return 1;  // Завершение программы с ошибкой
    }

    unsigned long long frequency[SYMBOL_COUNT] = {0};  // Массив для хранения частот символов
    int ch;  // Переменная для чтения символов
    while ((ch = fgetc(fin)) != EOF) frequency[(unsigned char)ch]++;  // Подсчет частот символов
    rewind(fin);  // Перемотка файла в начало

    Heap h = {.count = 0};  // Инициализация кучи
    for (int i = 0; i < SYMBOL_COUNT; ++i) {  // Создание узлов для символов с ненулевой частотой
        if (frequency[i]) {
            HuffNode* n = malloc(sizeof(HuffNode));  // Выделение памяти для нового узла
            n->value = (unsigned char)i;  // Установка значения символа
            n->count = frequency[i];  // Установка частоты символа
            n->left = n->right = NULL;  // Установка указателей на потомков
            heap_insert(&h, n);  // Вставка узла в кучу
        }
    }

    while (h.count > 1) {  // Построение дерева Хаффмана
        HuffNode *a = heap_remove_min(&h), *b = heap_remove_min(&h);  // Удаление двух минимальных элементов
        HuffNode* parent = malloc(sizeof(HuffNode));  // Выделение памяти для нового узла
        parent->value = 0;  // Установка значения символа
        parent->count = a->count + b->count;  // Установка частоты символа
        parent->left = a;  // Установка левого потомка
        parent->right = b;  // Установка правого потомка
        heap_insert(&h, parent);  // Вставка нового узла в кучу
    }

    HuffNode* root = h.count ? heap_remove_min(&h) : NULL;  // Получение корня дерева Хаффмана

    BitCode table[SYMBOL_COUNT] = {0};  // Массив для хранения битовых кодов
    char buffer[MAX_CODE_LEN];  // Буфер для битовых кодов
    if (root) assign_codes(root, table, buffer, 0);  // Присвоение кодов символам

    FILE *fout = fopen("encoded.bin", "wb");  // Открытие файла для записи
    if (!fout) {  // Если файл не открыт
        perror("encoded.bin");  // Вывод сообщения об ошибке
        return 2;  // Завершение программы с ошибкой
    }

    // Запись заголовка (частоты)
    int used = 0;  // Количество символов с ненулевой частотой
    for (int i = 0; i < SYMBOL_COUNT; ++i)
        if (frequency[i]) ++used;  // Подсчет символов с ненулевой частотой
    fwrite(&used, sizeof(int), 1, fout);  // Запись количества символов с ненулевой частотой
    for (int i = 0; i < SYMBOL_COUNT; ++i)
        if (frequency[i]) {  // Запись символов и их частот
            unsigned char c = (unsigned char)i;
            fwrite(&c, 1, 1, fout);
            fwrite(&frequency[i], sizeof(unsigned long long), 1, fout);
        }

    // Кодирование и запись
    unsigned char out_byte = 0;  // Байт для записи
    int bits_used = 0;  // Количество использованных бит
    while ((ch = fgetc(fin)) != EOF) {  // Кодирование каждого символа
        BitCode* b = &table[(unsigned char)ch];  // Получение битового кода символа
        for (int i = 0; i < b->length; ++i) {  // Запись каждого бита
            out_byte = (out_byte << 1) | (b->bits[i] - '0');  // Добавление бита к байту
            bits_used++;  // Увеличение количества использованных бит
            if (bits_used == 8) {  // Если байт заполнен
                fwrite(&out_byte, 1, 1, fout);  // Запись байта
                bits_used = 0;  // Сброс количества использованных бит
                out_byte = 0;  // Сброс байта
            }
        }
    }
    if (bits_used > 0) {  // Если остались неиспользованные биты
        out_byte <<= (8 - bits_used);  // Сдвиг байта для заполнения
        fwrite(&out_byte, 1, 1, fout);  // Запись байта
    }

    fclose(fin);  // Закрытие файла ввода
    fclose(fout);  // Закрытие файла вывода
    free_tree(root);  // Освобождение памяти дерева Хаффмана

    // Декодирование сразу
    fin = fopen("encoded.bin", "rb");  // Открытие файла для чтения
    fout = fopen("decoded.txt", "wb");  // Открытие файла для записи
    if (!fin || !fout) {  // Если файлы не открыты
        perror("decode I/O");  // Вывод сообщения об ошибке
        return 3;  // Завершение программы с ошибкой
    }

    fread(&used, sizeof(int), 1, fin);  // Чтение количества символов с ненулевой частотой
    memset(frequency, 0, sizeof(frequency));  // Сброс частот
    for (int i = 0; i < used; ++i) {  // Чтение символов и их частот
        unsigned char c;
        unsigned long long f;
        fread(&c, 1, 1, fin);
        fread(&f, sizeof(unsigned long long), 1, fin);
        frequency[c] = f;
    }

    h.count = 0;  // Сброс количества элементов в куче
    for (int i = 0; i < SYMBOL_COUNT; ++i) {  // Создание узлов для символов с ненулевой частотой
        if (frequency[i]) {
            HuffNode* n = malloc(sizeof(HuffNode));  // Выделение памяти для нового узла
            n->value = (unsigned char)i;  // Установка значения символа
            n->count = frequency[i];  // Установка частоты символа
            n->left = n->right = NULL;  // Установка указателей на потомков
            heap_insert(&h, n);  // Вставка узла в кучу
        }
    }

    while (h.count > 1) {  // Построение дерева Хаффмана
        HuffNode *a = heap_remove_min(&h), *b = heap_remove_min(&h);  // Удаление двух минимальных элементов
        HuffNode* parent = malloc(sizeof(HuffNode));  // Выделение памяти для нового узла
        parent->value = 0;  // Установка значения символа
        parent->count = a->count + b->count;  // Установка частоты символа
        parent->left = a;  // Установка левого потомка
        parent->right = b;  // Установка правого потомка
        heap_insert(&h, parent);  // Вставка нового узла в кучу
    }

    root = h.count ? heap_remove_min(&h) : NULL;  // Получение корня дерева Хаффмана
    HuffNode* current = root;  // Текущий узел дерева Хаффмана

    ch = 0;  // Символ для чтения
    bits_used = 0;  // Количество использованных бит
    while ((ch = fgetc(fin)) != EOF) {  // Декодирование каждого символа
        for (int i = 7; i >= 0; --i) {  // Чтение каждого бита
            int bit = (ch >> i) & 1;  // Получение бита
            current = bit ? current->right : current->left;  // Переход к потомку
            if (current && !current->left && !current->right) {  // Если текущий узел является листом
                fputc(current->value, fout);  // Запись символа
                current = root;  // Возврат к корню дерева
            }
        }
    }

    fclose(fin);  // Закрытие файла ввода
    fclose(fout);  // Закрытие файла вывода
    free_tree(root);  // Освобождение памяти дерева Хаффмана
    return 0;  // Завершение программы
}
