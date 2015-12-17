#ifndef list_h_
#define list_h_

#include <stdlib.h>
#include <memory.h>


#define LIST_ERR_SUCCESS             0
#define LIST_ERR_NOT_ENOUGH_MEMORY  -1
#define LIST_ERR_INVALID_PARAMETER  -2

//Structural amenities
typedef int (*COMPARE_FUNCTION) (void*, void*);

struct _LIST_NODE;
struct _LIST;

typedef struct _LIST LIST,*PLIST;
typedef struct _LIST_NODE LIST_NODE, *PLIST_NODE;

struct _LIST_NODE{
    void* data;
    int   dataSize;
    PLIST_NODE next;
};

struct _LIST{
    PLIST_NODE head;
    int        count;
};


//Add Modes
#define MODE_BEG    0
#define MODE_END    1

//Public Functions
int   list_initialize      (PLIST list);
int   list_add_node        (PLIST list, void* data, int dataSize, int addMode);
int   list_add_node_at     (PLIST list, void* data, int dataSize, int index);
int   list_add_node_sorted (PLIST list, void* data, int dataSize, COMPARE_FUNCTION func);
int   list_del_node        (PLIST list, int delMode);
int   list_del_node_at     (PLIST list, int index);
void* list_get_node        (PLIST list, int getMode);
void* list_get_node_at     (PLIST list, int index);
int   list_get_node_count  (PLIST list);





#endif

