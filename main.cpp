#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

size_t const INIT_SIZE = 16;
size_t const MAGNIFICATION_FACTOR = 2;
size_t const REDUCTION_FACTOR = 4;
size_t const NEW_LINE_INDICATOR = 10;

#define STACKASSERT(stk_) \
    do { StackAssertFunc(stk_, __FILE__, __LINE__, __func__); } while(0)
#define STACKCTOR(stk_) \
    do { StackCtor(stk_, #stk_, __FILE__, __LINE__, __func__); } while(0)
#define FREE(ptr_) \
    do { free(ptr_); ptr_ = NULL; } while(0)

enum ErrorCodes
{
    StackOK = 0,
    InvalidStructurePointer,
    InvalidStackPointer,
    IncorrectStackSize,
    IncorrectStackCapacity,
    SizeExceededCapacity,
};

typedef double StackElem_t;

struct Stack_info
{
    const char * pointer_name;
    const char * pointer_place;
    int pointer_line;
    const char * func_name;
};

struct Stack_t
{
    StackElem_t * data;
    size_t size;
    size_t capacity;
    Stack_info info;
};

void StackCtor(Stack_t * stk, const char * pointer_name, const char * pointer_place, int pointer_line,
               const char * func_name);
void StackDtor(Stack_t * stk);
void push(Stack_t * stk, StackElem_t value);
StackElem_t pop(Stack_t * stk);
void StackData(Stack_t * stk);
enum ErrorCodes StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func);
void StackDump(Stack_t * stk, const char * file, int line, const char * func);
enum ErrorCodes StackError(Stack_t * stk);

int main()
{
    Stack_t stk = {}; STACKCTOR(&stk);

    for (int i = 0; i < 30; i++) { push(&stk, 100 * (i + 1)); }

    pop(&stk);
    pop(&stk);

    StackData(&stk);
    StackDump(&stk, __FILE__, __LINE__, __func__);

    StackDtor(&stk);

    return 0;
}

void StackCtor(Stack_t * stk, const char * pointer_name, const char * pointer_place, int pointer_line,
               const char * func_name)
{
    stk -> data = (StackElem_t *)calloc(INIT_SIZE, sizeof(StackElem_t));
    STACKASSERT(stk);

    (stk -> info).pointer_name = pointer_name;
    (stk -> info).pointer_place = pointer_place;
    (stk -> info).pointer_line = pointer_line;
    (stk -> info).func_name = func_name;

    stk -> size = 0;
    stk -> capacity = INIT_SIZE;

    STACKASSERT(stk);
}

void StackDtor(Stack_t * stk)
{
    STACKASSERT(stk);
    memset(stk -> data, 0, stk -> capacity * sizeof(StackElem_t));
    FREE(stk -> data);
}

void push(Stack_t * stk, StackElem_t value)
{
    STACKASSERT(stk);
    if (stk -> size >= stk -> capacity)
    {
        stk -> capacity *= MAGNIFICATION_FACTOR;
        stk -> data = (StackElem_t *)realloc(stk -> data, stk -> capacity * sizeof(StackElem_t));
        STACKASSERT(stk);
    }
    *(stk -> data + stk -> size) = value;
    stk -> size++;
    STACKASSERT(stk);
}

StackElem_t pop(Stack_t * stk)
{
    STACKASSERT(stk);
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
    STACKASSERT(stk);

    FILE * data_file = fopen("data_file.txt", "w");
    if (!data_file)
    {
        fprintf(stderr, "File opening error\n");
        assert(0);
    }

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

enum ErrorCodes StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func)
{
    if (StackError(stk))
    {
        StackDump(stk, file, line, func);
        fprintf(stderr, "%s:%d in function: %s\n", file, line, func);
        assert(0);
    }
    return StackOK;
}

void StackDump(Stack_t * stk, const char * file, int line, const char * func)
{
    STACKASSERT(stk);

    FILE * dump_file = fopen("dump_file.txt", "w");
    if (!dump_file)
    {
        fprintf(stderr, "File opening error\n");
        assert(0);
    }

    fprintf(dump_file,
            "Stack_t [%p]\n"
            "called from %s: %d (%s)\n"
            "name %s born at %s: %d (%s)\n"
            "left kanareyka..."
            "right canareyka..."
            "capacity = ..."
            "size = ..."
            ".........",
            stk -> data,
            file, line, func,
            stk -> info.pointer_name, stk -> info.pointer_place, stk -> info.pointer_line, stk -> info.func_name);
    fclose(dump_file);
    dump_file = NULL;
}

enum ErrorCodes StackError(Stack_t * stk)
{
    if (stk == NULL)                    { return InvalidStructurePointer ;}
    if (stk -> data == NULL)            { return InvalidStackPointer     ;}
    if (stk -> size < 0)                { return IncorrectStackSize      ;}
    if (stk -> capacity < 0)            { return IncorrectStackCapacity  ;}
    if (stk -> size > stk -> capacity)  { return SizeExceededCapacity    ;}
                                        { return StackOK                 ;}
}
