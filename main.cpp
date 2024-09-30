#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef double StackElem_t;

size_t      const INIT_SIZE            =        16;
size_t      const MAGNIFICATION_FACTOR =         2;
size_t      const REDUCTION_FACTOR     =         4;
size_t      const NEW_LINE_INDICATOR   =        10;
StackElem_t const CANARY_VALUE         = 0xDEFACED;

#define STACKASSERT(stk_) \
    do { StackAssertFunc(stk_, __FILE__, __LINE__, __func__); } while(0)
#define STACKCTOR(stk_) \
    do { StackCtor(stk_, #stk_, __FILE__, __LINE__, __func__); } while(0)
#define FREE(ptr_) \
    do { free(ptr_); ptr_ = NULL; } while(0)
#define $ fprintf(stderr, "%s:%d in function: %s\n", __FILE__, __LINE__, __func__);

enum ErrorCodes
{
    StackOK = 0,
    InvalidStructurePointer,
    InvalidStackPointer,
    IncorrectStackSize,
    IncorrectStackCapacity,
    SizeExceededCapacity,
    LeftAttackOnStructure,
    RightAttackOnStructure,
    LeftAttackOnStack,
    RightAttackOnStack,
};

struct Stack_info
{
    const char * pointer_name;
    const char * pointer_place;
    int pointer_line;
    const char * func_name;
};

struct Stack_t
{
    int left_canary;
    StackElem_t * data;
    size_t size;
    size_t capacity;
    Stack_info info;
    int right_canary;
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

    for (int i = 0; i < 100; i++) { push(&stk, 100 * (i + 1)); }
    for (int i = 0; i < 80; i++) { pop(&stk); }

    StackData(&stk);
    StackDump(&stk, __FILE__, __LINE__, __func__);

    StackDtor(&stk);

    return 0;
}

void StackCtor(Stack_t * stk, const char * pointer_name, const char * pointer_place, int pointer_line,
               const char * func_name)
{
    stk -> data = (StackElem_t *)calloc(INIT_SIZE + 2, sizeof(StackElem_t));

    stk -> size = 0;
    stk -> capacity = INIT_SIZE;

    stk -> data[0] = CANARY_VALUE;
    stk -> data[stk -> capacity + 1] = CANARY_VALUE;

    stk -> left_canary  = CANARY_VALUE;
    stk -> right_canary = CANARY_VALUE;

    stk -> info.pointer_name  =  pointer_name;
    stk -> info.pointer_place = pointer_place;
    stk -> info.pointer_line  =  pointer_line;
    stk -> info.func_name     =     func_name;

    STACKASSERT(stk);
}

void StackDtor(Stack_t * stk)
{
    STACKASSERT(stk);

    memset(stk -> data, 0, (stk -> capacity + 2) * sizeof(StackElem_t));
    FREE(stk -> data);
}

void push(Stack_t * stk, StackElem_t value)
{
    STACKASSERT(stk);

    if (stk -> size == stk -> capacity)
    {
        stk -> capacity *= MAGNIFICATION_FACTOR;
        stk -> data = (StackElem_t *)realloc(stk -> data, (stk -> capacity + 2) * sizeof(StackElem_t));
        memset(stk -> data + stk -> size + 1, 0, (stk -> capacity - stk -> size) * sizeof(StackElem_t));
        stk -> data[0] = CANARY_VALUE;
        stk -> data[stk -> capacity + 1] = CANARY_VALUE;
        STACKASSERT(stk);
    }
    *(stk -> data + stk -> size + 1) = value;
    stk -> size++;

    STACKASSERT(stk);
}

StackElem_t pop(Stack_t * stk)
{
    STACKASSERT(stk);

    stk -> size--;
    StackElem_t value = *(stk -> data + stk -> size + 1);
    *(stk -> data + stk -> size + 1) = 0;
    if ((stk -> size != 0) && (stk -> capacity / stk -> size >= REDUCTION_FACTOR))
    {
        memset(stk -> data + stk -> size, 0, (stk -> capacity - stk -> size + 1) * sizeof(StackElem_t));
        stk -> data = (StackElem_t *)realloc(stk -> data, (stk -> size + 2) * sizeof(StackElem_t));

        stk -> capacity /= REDUCTION_FACTOR;

        stk -> data[0] = CANARY_VALUE;
        stk -> data[stk -> capacity + 1] = CANARY_VALUE;
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
    for (size_t i = 0; i < stk -> capacity + 2; i++)
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
        memset(stk -> data, 0, (stk -> capacity + 2) * sizeof(StackElem_t));
        assert(0);
    }
    return StackOK;
}

void StackDump(Stack_t * stk, const char * file, int line, const char * func)
{
    FILE * dump_file = fopen("dump_file.txt", "w");
    if (!dump_file)
    {
        fprintf(stderr, "File opening error\n");
        assert(0);
    }

    fprintf(dump_file,
            "Stack_t [%p]\n"
            "    called from %s: %d (%s)\n"
            "    name %s born at %s: %d (%s)\n"
            "    {\n"
            "        left canary = 0x%X\n"
            "        right canary = 0x%X\n"
            "        capacity = %d\n"
            "        size     = %d\n"
            "        data [%p]\n"
            "        {\n",
            stk,
            file, line, func,
            stk -> info.pointer_name, stk -> info.pointer_place, stk -> info.pointer_line, stk -> info.func_name,
            (unsigned)stk -> data[0],
            (unsigned)stk -> data[stk -> capacity + 1],
            stk -> capacity,
            stk -> size,
            stk -> data);

    fclose(dump_file);
    dump_file = NULL;
}

enum ErrorCodes StackError(Stack_t * stk)
{
    if (stk == NULL)                                      { return InvalidStructurePointer ;}
    if (stk -> data == NULL)                              { return InvalidStackPointer     ;}
    if (stk -> size < 0)                                  { return IncorrectStackSize      ;}
    if (stk -> capacity < 0)                              { return IncorrectStackCapacity  ;}
    if (stk -> size > stk -> capacity)                    { return SizeExceededCapacity    ;}
    if (stk -> left_canary != CANARY_VALUE)               { return LeftAttackOnStructure   ;}
    if (stk -> right_canary != CANARY_VALUE)              { return RightAttackOnStructure  ;}
    if (stk -> data[0] != CANARY_VALUE)                   { return LeftAttackOnStack       ;}
    if (stk -> data[stk -> capacity + 1] != CANARY_VALUE) { return RightAttackOnStack      ;}
                                                          { return StackOK                 ;}
}
