#include "list.h"

int list_initialize(PLIST list){
    list->head = NULL;
    list->count = 0;
    return LIST_ERR_SUCCESS;
}



int list_add_node(PLIST list, void* data, int dataSize, int addMode){
    PLIST_NODE node = (PLIST_NODE)malloc(sizeof(LIST_NODE));
    if (node){
        node->next = NULL;
        node->data = malloc(dataSize);
        if (node->data){
            memcpy(node->data,data,dataSize);
            node->dataSize = dataSize;

            if (MODE_BEG == addMode){
                node->next = list->head;
                list->head = node;
            }else{//MODE_END == addMode
                if (!list->head){
                    list->head = node;
                }else{
                    PLIST_NODE curNode = list->head;
                    while (curNode->next != NULL)
                        curNode = curNode->next;
                    curNode->next = node;
                }
            }			
            ++list->count;
        }else{
            free(node);
            return LIST_ERR_NOT_ENOUGH_MEMORY;
        }
    }else{
        return LIST_ERR_NOT_ENOUGH_MEMORY;
    }
    
    return LIST_ERR_SUCCESS;
}
int list_add_node_at(PLIST list, void* data, int dataSize, int index){
    if (index == 0){
        //Add to beginning
        return list_add_node(list,data,dataSize,MODE_BEG);
    }else if (index == list->count){
        //Add to end
        return list_add_node(list,data,dataSize,MODE_END);
    }else if (index < list->count && index > 0){
        //Add to somewhere in between
        int i;
        PLIST_NODE newNode  = NULL;
        PLIST_NODE prevNode = NULL;
        PLIST_NODE curNode  = list->head;
        
        for (i=0;i < index;++i){
            prevNode = curNode;
            curNode = curNode->next;
        }

        newNode = (PLIST_NODE)malloc(sizeof(LIST_NODE));
        if (newNode){
            newNode->data = malloc(dataSize);
            if (newNode->data){
                memcpy(newNode->data,data,dataSize);
                newNode->dataSize = dataSize;				
                prevNode->next = newNode;
                newNode->next = curNode;
                list->count++;
            }else{
                //malloc failed to allocate newNode->data
                return LIST_ERR_NOT_ENOUGH_MEMORY;
            }
        }else{
            //malloc failed to allocate newNode
            return LIST_ERR_NOT_ENOUGH_MEMORY;
        }

    }else{
        //index is not in a valid range
        return LIST_ERR_INVALID_PARAMETER;
    }
    return LIST_ERR_SUCCESS;
}

int list_add_node_sorted(PLIST list, void* data, int dataSize, COMPARE_FUNCTION func){
    PLIST_NODE node = (PLIST_NODE)malloc(sizeof(LIST_NODE));
    if (node){
        node->next = NULL;
        node->data = malloc(dataSize);
        if (node->data){
            memcpy(node->data,data,dataSize);
            node->dataSize = dataSize;

            if (!list->head){
                list->head = node;
            }else{
                PLIST_NODE prevNode=NULL;
                PLIST_NODE curNode = list->head;
                while (curNode != NULL){
                    if (func(node->data, curNode->data) <0)
                        break;
                    prevNode = curNode;
                    curNode = curNode->next;
                }
                if (prevNode->next)
                    prevNode->next = node;
                node->next = curNode;
            }
            ++list->count;
        }else{
            free(node);
            return LIST_ERR_NOT_ENOUGH_MEMORY;
        }
    }else{
        return LIST_ERR_NOT_ENOUGH_MEMORY;
    }
    return LIST_ERR_SUCCESS;
}

int list_del_node(PLIST list, int delMode){
    if (list->head){
        //Case: at least 1 element in the list
        if (MODE_BEG == delMode){
            PLIST_NODE cur = list->head;
            list->head = list->head->next;
            free(cur);
        }else{//MODE_END == delMode		
            if (list->head->next == NULL){
                //Case: exactly 1 element in the list
                free (list->head);
                list->head = 0;
            }else{
                //Case: more than 1 element in the list
                PLIST_NODE cur = list->head;
                PLIST_NODE prev = NULL;
                while (cur->next != NULL){
                    prev = cur;
                    cur = cur->next;
                }
                prev->next = NULL;
                free(cur);
            }
        }
        --list->count;
    }
    
    return LIST_ERR_SUCCESS;
}

int list_del_node_at(PLIST list, int index){

    PLIST_NODE prev,cur;
    int i;

    if (list->count == 0 || index < 0 || index >= list->count){
        //the list is empty
        return LIST_ERR_INVALID_PARAMETER;
    }

    if (index == 0){
        //delete the first element
        list_del_node(list, MODE_BEG);
        return LIST_ERR_SUCCESS;
    }else if (index == list->count-1){
        list_del_node(list, MODE_END);
    }else{
        //Not the beginning or the end.
        //And there is at least one elements
        cur = list->head;
        i = 0;
        for (i = 0;i < index;++i){
            prev = cur;
            cur = cur->next;
        }
        prev->next = cur->next;
        free (cur);
        --list->count;
    }
    return LIST_ERR_SUCCESS;
}

void* list_get_node(PLIST list, int getMode){
    if (list->count){
        if (MODE_BEG == getMode){
            return list->head->data;
        }else{//MODE_END == getMod
            PLIST_NODE curNode = list->head;
            while (curNode->next != NULL)
                curNode=curNode->next;
            return curNode->data;
        }
    }else{
        //List is empty, nothing to return.
        return NULL;
    }
}
void* list_get_node_at(PLIST list, int index){

    return NULL;
}
int list_get_node_count(PLIST list){
    return list->count;
}