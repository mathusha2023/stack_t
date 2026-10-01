#include "hash.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "stack.h"
#include "log.h"

/*
Здесь был страшный баг под названием "Я ЗАБЫЛ РАЗЫМЕНОВАТЬ УКАЗАТЕЛЬ БЛ"
Но благодаря Нагаеву Тамерлану (github: https://github.com/Tamik13) удалось отловить
эту мерзость без глубоких моральных увечий, огромное спасибо!
*/

// Djb2 hash function
size_t hash(const char *s, size_t size)
{
    assert(s);
    assert(size > 0);

    size_t hash = 5381;

    for (size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + (size_t)s[i]; // hash * 33 + curr

    return hash % NUM_BUCKETS;
}
