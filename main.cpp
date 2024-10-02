#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "stack.h"

int main()
{
    Stack_t stk = {};
    STACKCTOR(&stk);

    for (int i = 0; i < 100; i++) { push(&stk, 100 * (i + 1)); }
    for (int i = 0; i < 80; i++) { pop(&stk); }

    StackData(&stk);
    StackDump(&stk, __FILE__, __LINE__, __func__);

    StackDtor(&stk);

    return 0;
}
