#include "ring_buffer.h"
#include <stdlib.h>
#include <memory.h>

int rb_initialize(PRING_BUFFER prb, int capacity)
{
    prb->buffer     = (DATUM*)malloc(capacity*sizeof(DATUM));
    prb->capacity   = capacity;
    prb->used       = 0;
    prb->start      = 0;

    return 0;
}

int rb_deinitialize(PRING_BUFFER prb)
{
    free(prb->buffer);
    prb->buffer     = 0;
    prb->capacity   = 0;
    prb->used       = 0;
    prb->start      = 0;
    return 0;
}
int rb_get_used(PRING_BUFFER prb)
{
    return prb->used;
}

int rb_get_capacity(PRING_BUFFER prb)
{
    return prb->capacity;
}

int rb_is_full(PRING_BUFFER prb)
{
    return !(rb_get_capacity(prb) - rb_get_used(prb));
}

int rb_add_data(PRING_BUFFER prb, DATUM* d, int count)
{
    int i;

    if (prb->used+count <= prb->capacity)
    {
        for (i = 0; i < count;++i)
        {
            prb->buffer[(prb->start+prb->used) % prb->capacity] = d[i];
            prb->used++;
        }
    }
    return 0;
}

int rb_remove_data(PRING_BUFFER prb, int count)
{
    if (prb->used >= count)
    {
        prb->start = (prb->start + count) % prb->capacity;
        prb->used -= count;
    }
    return 0;
}
int rb_get_data(PRING_BUFFER prb, DATUM* d, int count)
{
    int i;
    if (prb->used >= count)
    {
        for (i = 0;i < count;++i)
            d[i] = prb->buffer[(prb->start+i) % prb->capacity];
    }
    return 0;
}

