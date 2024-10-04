#include "stack.h"

#include <stdlib.h>
#include <assert.h>

#include "hashing.h"

void StackCtor(Stack_t * stk, const char * pointer_name, const char * pointer_place, int pointer_line,
               const char * func_name)
{
    if (!stk)
    {
        fprintf(stderr, "Invalid pointer to the struct\n");
        assert(0);
    }

    stk->data = (StackElem_t *)calloc(INIT_SIZE + 2, sizeof(StackElem_t));
    if (!stk->data)
    {
        fprintf(stderr, "Invalid allocation\n");
        assert(0);
    }

    stk->size = 0;
    stk->capacity = INIT_SIZE;

    memset(stk->data, POISON, (stk->capacity + 2) * sizeof(StackElem_t));

    stk->data[0] = CANARY_VALUE;
    stk->data[stk->capacity + 1] = CANARY_VALUE;

    stk->left_canary  = CANARY_VALUE;
    stk->right_canary = CANARY_VALUE;

    stk->info.pointer_name  =  pointer_name;
    stk->info.pointer_place = pointer_place;
    stk->info.pointer_line  =  pointer_line;
    stk->info.func_name     =     func_name;

    stk->hash_of_stack = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);

    stk->hash_of_struct = 0;
    stk->hash_of_struct = murmur3_32((uint8_t *)stk, sizeof(stk), 52);

    STACKASSERT(stk);
}

void StackDtor(Stack_t * stk)
{
    STACKASSERT(stk);
    memset(stk->data, 0, (stk->capacity + 2) * sizeof(StackElem_t));
    FREE(stk->data);
}

void push(Stack_t * stk, StackElem_t value)
{
    STACKASSERT(stk);

    if ((long unsigned)stk->size == stk->capacity)
    {
        stk->data[stk->capacity + 1] = 0;
        stk->capacity *= MAGNIFICATION_FACTOR;

        stk->data = (StackElem_t *)realloc(stk->data, (stk->capacity + 2) * sizeof(StackElem_t));
        if (!stk->data)
        {
            fprintf(stderr, "Invalid allocation\n");
            assert(0);
        }
        memset(stk->data + stk->size + 1, POISON, (stk->capacity - (long unsigned)stk->size) * sizeof(StackElem_t));

        stk->data[stk->capacity + 1] = CANARY_VALUE;
        stk->hash_of_stack = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);

        stk->hash_of_struct = 0;
        stk->hash_of_struct = murmur3_32((uint8_t *)stk, sizeof(stk), 52);
        STACKASSERT(stk);
    }
    stk->data[stk->size + 1] = value;
    stk->size++;
    stk->hash_of_stack = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);

    STACKASSERT(stk);
}

StackElem_t pop(Stack_t * stk)
{
    STACKASSERT(stk);
    stk->size--;
    STACKASSERT(stk);

    StackElem_t value = stk->data[stk->size + 1];
    stk->data[stk->size + 1] = POISON;

    stk->hash_of_stack = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);
    stk->hash_of_struct = 0;
    stk->hash_of_struct = murmur3_32((uint8_t *)stk, sizeof(stk), 52);

    if ((stk->size != 0) && (stk->capacity / (long unsigned)stk->size >= REDUCTION_FACTOR) && stk->capacity > INIT_SIZE)
    {
        memset(stk->data + stk->size, 0, (stk->capacity - (long unsigned)stk->size + 1) * sizeof(StackElem_t));
        stk->data = (StackElem_t *)realloc(stk->data, (long unsigned)(stk->size + 2) * sizeof(StackElem_t));
        if (!stk->data)
        {
            fprintf(stderr, "Invalid allocation\n");
            assert(0);
        }

        stk->capacity /= REDUCTION_FACTOR;

        stk->data[stk->capacity + 1] = CANARY_VALUE;

        stk->hash_of_stack = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);
        stk->hash_of_struct = 0;
        stk->hash_of_struct = murmur3_32((uint8_t *)stk, sizeof(stk), 52);
    }
    STACKASSERT(stk);

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
                       "SIZE: %lld\n"
                       "CAPACITY: %lu\n\n", stk->data, stk->size, stk->capacity);
    for (size_t i = 0; i < stk->capacity; i++)
    {
        if (i % NEW_LINE_INDICATOR == 0)
        {
            putc('\n', data_file);
            fprintf(data_file, "[%p]     ", stk->data + i + 1);
        }
        fprintf(data_file, "%-12lg",*(stk->data + i + 1));
    }

    FCLOSE(data_file);
}

ErrorCodes StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func)
{
    if (!StackError(stk)) { return StackOK; }

    StackDump(stk, file, line, func);
    fprintf(stderr, "%s:%d in function: %s\n", file, line, func);
    memset(stk->data, 0, (stk->capacity + 2) * sizeof(StackElem_t));
    assert(0);
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
            "    called from %s: %d (%s)\n"
            "    name %s born at %s: %d (%s)\n"
            "    {\n"
            "        left canary = 0x%X\n"
            "        right canary = 0x%X\n"
            "        capacity = %lu\n"
            "        size     = %lld\n"
            "        data [%p]\n"
            "        {\n",
            stk,
            file, line, func,
            stk->info.pointer_name, stk->info.pointer_place, stk->info.pointer_line, stk->info.func_name,
            (unsigned)stk->data[0],
            (unsigned)stk->data[stk->capacity + 1],
            stk->capacity,
            stk->size,
            stk->data);
    for(int i = 0; i < stk->size; i++)
    {
        fprintf(dump_file, "            *[%d] = %lg\n", i, stk->data[i+1]);
    }
    for(int i = 0; i < stk->capacity - stk->size; i++)
    {
        fprintf(dump_file, "             [%lld] = %lg (POISON)\n", stk->size + i, stk->data[stk->size+i+1]);
    }
    fprintf(dump_file, "        }\n    }\n");

    FCLOSE(dump_file);
}

ErrorCodes StackError(Stack_t * stk)
{
    if (stk == NULL)                                   { return InvalidStructurePointer ;}
    if (stk->data == NULL)                             { return InvalidStackPointer     ;}
    if (stk->size < 0)                                 { return IncorrectStackSize      ;}
    if ((long unsigned)stk->size > stk->capacity)      { return SizeExceededCapacity    ;}

    uint32_t current_stack_hash = murmur3_32((uint8_t *)stk->data, (stk->capacity + 2) * sizeof(StackElem_t), 52);
    uint32_t expected_struct_hash = stk->hash_of_struct;
    uint32_t current_struct_hash = murmur3_32((uint8_t *)stk, sizeof(stk), 52);

    if (stk->left_canary != CANARY_VALUE)              { return LeftAttackOnStructure   ;}
    if (stk->right_canary != CANARY_VALUE)             { return RightAttackOnStructure  ;}
    if (stk->data[0] != CANARY_VALUE)                  { return LeftAttackOnStack       ;}
    if (stk->data[stk->capacity + 1] != CANARY_VALUE)  { return RightAttackOnStack      ;}
    if (current_stack_hash != stk->hash_of_stack)      { return IncorrectHashOfStack    ;}
    if (current_struct_hash != expected_struct_hash )  { return IncorrectHashOfStruct   ;}
                                                       { return StackOK                 ;}
}
