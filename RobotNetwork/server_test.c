#include "robot_network.h"
#include <stdio.h>

RN_CONTEXT context_server;

void* event_handler_server(RN_PCONTEXT pcontext, PSOCK psrc_socket, int event_code, int err_code, int nid, void* data, int size)
{
    switch (event_code)
    {
    case EVENT_NETWORK_CONNECTION_NEW:
        printf("\nNode %i: Connected\n", nid);
        break;
    case EVENT_NETWORK_CONNECTION_CLOSED:
        printf("Node %i: Disconnected\n",nid);
        break;
    case EVENT_NETWORK_CONNECTION_RX:
    {
        int i;
        printf("Node %i: Recieved %i bytes from Node\n", nid, size);
        for (i = 0;i < size;++i)
        {
            printf("%i ", ((char*)data+i));
        }
        printf("\n");
        break;
    }
    case EVENT_NETWORK_CONNECTION_TX:
        printf("Node %i: Sent %i bytes to Node\n", nid, size);
        break;
    case EVENT_NETWORK_CONNECTION_TX_RDY:
        //printf("Node %i: Can Send\n", nid, size); //Very Verbose
        break;
    case EVENT_NETWORK_ERR:
        switch(err_code)
        {
            case ERR_NOT_IMPLEMENTED:
                break;
            case ERR_FAILEDTOCREATESOCKET:
                break;
            case ERR_FAILEDTOBINDSOCKET:
                break;
            case ERR_FAILEDTOLISTENSOCKET:
                break;
            case ERR_FAILEDTOCONNECTSOCKET:
                break;
            case ERR_FAILEDTOACCEPTSOCKET:
                break;
            case ERR_FAILEDTOWRITESOCKET:
                break;
            case ERR_FAILEDTOREADSOCKET:
                break;
            case ERR_FAILEDTOALLOCATEMEMORY:
                break;
            default:
                break;
        }
        break;
    default:
        printf("Unknown event.\n");
        break;
    }
    return NULL;
}



int main(int argc, char* argv[])
{
    printf("Server Test\n");
    rn_initialize();
    rn_start_server(&context_server, "127.0.0.1", 2333, event_handler_server);

    system("pause");

    rn_stop_server(&context_server);
    rn_deinitialize();
    
    return 0;
}
