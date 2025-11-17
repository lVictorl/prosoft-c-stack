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