#include "cstack.h"
#include <stdlib.h>
#include <string.h>

// Узел стека с гибким массивом для данных
typedef struct node_s
{
    struct node_s *prev; // Указатель на предыдущий элемент стека
    unsigned int size;   // Размер данных в узле
    char data[];         // Гибкий массив для хранения данных
} node_t;

// Запись в таблице стеков
typedef struct stack_entry_s
{
    int reserved;  // Флаг занятости слота (1 - занят, 0 - свободен)
    node_t *stack; // Указатель на вершину стека
} stack_entry_t;

// Глобальная таблица стеков
static struct
{
    unsigned int size;      // Текущий размер таблицы
    stack_entry_t *entries; // Динамический массив записей
} g_table = {0, NULL};

// Вспомогательная функция для расширения таблицы стеков
static int extend_table(void)
{
    // Определяем новый размер таблицы: начальный размер 10, затем удвоение
    unsigned int new_size = g_table.size == 0 ? 10 : g_table.size * 2;
    stack_entry_t *new_entries = (stack_entry_t *)realloc(g_table.entries,
                                                          new_size * sizeof(stack_entry_t));
    if (!new_entries)
    {
        return 0; // Ошибка выделения памяти
    }

    // Инициализация новых записей как свободных
    for (unsigned int i = g_table.size; i < new_size; i++)
    {
        new_entries[i].reserved = 0;
        new_entries[i].stack = NULL;
    }

    g_table.entries = new_entries;
    g_table.size = new_size;
    return 1;
}

// Создание нового стека
hstack_t stack_new(void)
{
    // Поиск первого свободного слота в таблице
    for (unsigned int i = 0; i < g_table.size; i++)
    {
        if (!g_table.entries[i].reserved)
        {
            g_table.entries[i].reserved = 1;
            g_table.entries[i].stack = NULL;
            return (hstack_t)i;
        }
    }

    // Если свободных слотов нет - расширяем таблицу
    if (!extend_table())
    {
        return -1; // Ошибка расширения таблицы
    }

    // Используем последний созданный слот
    unsigned int new_index = g_table.size - 1;
    g_table.entries[new_index].reserved = 1;
    g_table.entries[new_index].stack = NULL;
    return (hstack_t)new_index;
}

// Проверка валидности хэндлера стека
int stack_valid_handler(const hstack_t stack)
{
    // Хэндлер должен быть в пределах таблицы и слот должен быть занят
    if (stack < 0 || (unsigned int)stack >= g_table.size)
    {
        return 1;
    }
    return !g_table.entries[stack].reserved;
}

// Получение количества элементов в стеке
unsigned int stack_size(const hstack_t stack)
{
    if (stack_valid_handler(stack))
    {
        return 0;
    }

    // Линейный подсчет элементов обходом от вершины
    unsigned int count = 0;
    node_t *current = g_table.entries[stack].stack;
    while (current != NULL)
    {
        count++;
        current = current->prev;
    }
    return count;
}

// Добавление элемента в стек
void stack_push(const hstack_t stack, const void *data_in, const unsigned int size)
{
    // Проверка валидности параметров
    if (stack_valid_handler(stack) || data_in == NULL || size == 0)
    {
        return;
    }

    // Выделение памяти для узла и данных в одном блоке
    node_t *new_node = (node_t *)malloc(sizeof(node_t) + size);
    if (!new_node)
    {
        return; // Ошибка выделения памяти
    }

    // Инициализация нового узла
    new_node->prev = g_table.entries[stack].stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    // Обновление вершины стека
    g_table.entries[stack].stack = new_node;
}

// Извлечение элемента из стека
unsigned int stack_pop(const hstack_t stack, void *data_out, const unsigned int size)
{
    // Проверка возможности извлечения
    if (stack_valid_handler(stack) || g_table.entries[stack].stack == NULL)
    {
        return 0;
    }

    node_t *top = g_table.entries[stack].stack;

    // Определение размера копируемых данных (минимум из доступного и запрошенного)
    unsigned int copy_size = (size < top->size) ? size : top->size;

    // Копирование данных в буфер, если он предоставлен
    if (data_out != NULL && copy_size > 0)
    {
        memcpy(data_out, top->data, copy_size);
    }

    // Обновление вершины стека и освобождение памяти
    g_table.entries[stack].stack = top->prev;
    free(top);

    return copy_size;
}

// Освобождение стека и связанной с ним памяти
void stack_free(const hstack_t stack)
{
    if (stack_valid_handler(stack))
    {
        return; // Некорректный хэндлер
    }

    // Последовательное освобождение всех узлов стека
    node_t *current = g_table.entries[stack].stack;
    while (current != NULL)
    {
        node_t *prev = current->prev;
        free(current);
        current = prev;
    }

    // Помечаем слот как свободный
    g_table.entries[stack].reserved = 0;
    g_table.entries[stack].stack = NULL;
}