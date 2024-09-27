#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int const INIT_SIZE = 16;
int const MAGNIFICATION_FACTOR = 2;
int const REDUCTION_FACTOR = 4;
int const NEW_LINE_INDICATOR = 10;

typedef double StackElem_t;

struct Stack_t
{
    StackElem_t * data;
    size_t size;
    size_t capacity;
};

void StackCtor(Stack_t * stk);
void StackDtor(Stack_t * stk);
void push(Stack_t * stk, StackElem_t value);
StackElem_t pop(Stack_t * stk);
void StackData(Stack_t * stk);
void StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func);
void StackDump(Stack_t * stk);

int main()
{
    Stack_t stk = {};

    StackCtor(&stk);

    for (int i = 0; i < 30; i++) { push(&stk, 100 * (i + 1)); }

    pop(&stk);

    StackData(&stk);

    StackDtor(&stk);

    return 0;
}

void StackCtor(Stack_t * stk) // TODO type int
{
    // TODO check for NULL
    stk -> data = (StackElem_t *)calloc(INIT_SIZE, sizeof(StackElem_t));
    // TODO check for correct allocation
    stk -> size = 0;
    stk -> capacity = INIT_SIZE;
}

void StackDtor(Stack_t * stk)
{
    free(stk -> data);
    stk -> data = NULL;
}

void push(Stack_t * stk, StackElem_t value)
{
    if (stk -> size >= stk -> capacity)
    {
        stk -> capacity *= MAGNIFICATION_FACTOR;
        stk -> data = (StackElem_t *)realloc(stk -> data, stk -> capacity * sizeof(StackElem_t));
        // TODO check for correct allocation
    }
    *(stk -> data + stk -> size) = value;
    stk -> size++;
}

StackElem_t pop(Stack_t * stk)
{
    // TODO check for null size of stack
    stk -> size--;
    StackElem_t value = *(stk -> data + stk -> size);
    *(stk -> data + stk -> size) = 0;
    if ((stk -> size != 0) && (stk -> capacity / stk -> size >= REDUCTION_FACTOR))
    {
        memset(stk -> data + stk -> size, 0, (stk -> capacity - stk -> size) * sizeof(StackElem_t));
        stk -> data = (StackElem_t *)realloc(stk -> data, stk -> size * sizeof(StackElem_t));
        stk -> capacity /= REDUCTION_FACTOR;
    }
    return value;
}

void StackData(Stack_t * stk)
{
    FILE * data_file = fopen("data_file.txt", "w");
    // check for correct opening
    fprintf(data_file, "STACK DATA\n\n"
                       "STACK: [%p]\n"
                       "SIZE: %lu\n"
                       "CAPACITY: %lu\n", stk -> data, stk -> size, stk -> capacity);
    for (size_t i = 0; i < stk -> size; i++)
    {
        if (i % NEW_LINE_INDICATOR == 0)
        {
            putc('\n', data_file);
            fprintf(data_file, "[%-12p]     ", stk -> data + i);
        }
        fprintf(data_file, "%-12lg",*(stk -> data + i));
    }

    fclose(data_file);
    data_file = NULL;
}

void StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func)
{
    if (!stk -> data)
    {
        StackDump(stk);
        fprintf(stderr, "%s:%d in function: %s\n", file, line, func);
        assert(0);
    }
}

void StackDump(Stack_t * stk)
{
    FILE * dump_file = fopen("dump_file.txt", "w");
    // check for correct opening
    fprintf(dump_file, "DUMP STACK\n\n"
                       "STACK: [%p]\n"
                       "SIZE: %lu\n"
                       "CAPACITY: %lu\n", stk -> data, stk -> size, stk -> capacity);
    fclose(dump_file);
    dump_file = NULL;
}
