#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

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
    int left_canary;
    StackElem_t * data;
    long long size;
    size_t capacity;
    Stack_info info;
    uint32_t hash_of_stack;
    uint32_t hash_of_struct;
    int right_canary;
};

#define STACKCTOR(stk_) \
    do { StackCtor(stk_, #stk_, __FILE__, __LINE__, __func__); } while(0)
#define STACKASSERT(stk_) \
    do { StackAssertFunc(stk_, __FILE__, __LINE__, __func__); } while(0)
#define FREE(ptr_) \
    do { free(ptr_); ptr_ = NULL; } while(0)
#define FCLOSE(file_) \
    do { fclose(file_); file_ = NULL; } while(0)

static const uint64_t CANARY_VALUE  = 0xDEFACED;
static const size_t   INIT_SIZE            = 16;
static const size_t   MAGNIFICATION_FACTOR =  2;
static const size_t   REDUCTION_FACTOR     =  4;
static const size_t   NEW_LINE_INDICATOR   = 10;

enum ErrorCodes
{
    StackOK = 0,
    InvalidStructurePointer,
    InvalidStackPointer,
    IncorrectStackSize,
    SizeExceededCapacity,
    LeftAttackOnStructure,
    RightAttackOnStructure,
    LeftAttackOnStack,
    RightAttackOnStack,
    IncorrectHashOfStack,
    IncorrectHashOfStruct,
};

void StackCtor(Stack_t * stk, const char * pointer_name, const char * pointer_place, int pointer_line,
               const char * func_name);
void StackDtor(Stack_t * stk);
void push(Stack_t * stk, StackElem_t value); // FIXME return error code
StackElem_t pop(Stack_t * stk);
#define $ fprintf(stderr, "%s:%d in function: %s\n", __FILE__, __LINE__, __func__);
void StackData(Stack_t * stk);
ErrorCodes StackAssertFunc(Stack_t * stk, const char * file, int line, const char * func);
void StackDump(Stack_t * stk, const char * file, int line, const char * func);
ErrorCodes StackError(Stack_t * stk);

#endif // STACK_H
