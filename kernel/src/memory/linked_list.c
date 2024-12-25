#include "linked_list.h"
#include <heap.h>
#include <memory.h>

/*
    Linked list: Just structs pointing to each other
*/

void* LinkedListAllocate(void** LLfirstPtr, size_t structSize)
{
    LLHeader* target = (LLHeader*)malloc(structSize);
    memset(target, 0, structSize);

    LLHeader* current = (LLHeader*)(LLfirstPtr);

    while (true)
    {
        if (current == 0)
        {
            // this is our first one
            *LLfirstPtr = target;
            break;
        }

        if (current->next == 0)
        {
            // End of the linked list
            current->next = target;
            break;
        }

        current = current->next;
    }

    target->next = 0;
    return target;
}

