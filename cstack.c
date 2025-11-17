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