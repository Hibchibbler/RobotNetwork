#ifndef ring_buffer_h_
#define ring_buffer_h_

typedef unsigned char DATUM;

typedef struct _RING_BUFFER
{
    DATUM* buffer;
    int capacity;
    int used;
    int start;

}RING_BUFFER,*PRING_BUFFER;

int rb_initialize(PRING_BUFFER prb, int capacity);
int rb_deinitialize(PRING_BUFFER prb);
int rb_get_used(PRING_BUFFER prb);
int rb_get_capacity(PRING_BUFFER prb);
int rb_is_full(PRING_BUFFER prb);
int rb_remove_data(PRING_BUFFER prb, int count);
int rb_add_data(PRING_BUFFER prb, DATUM* d, int count);
int rb_get_data(PRING_BUFFER prb, DATUM* d, int count);

#endif
