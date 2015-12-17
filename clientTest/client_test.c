#include "..\RobotNetwork\robot_network.h"
#include <stdio.h>

#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_PORT     2333
#define CLIENT_NODE_ID  1

RN_CONTEXT context_client;

void* event_handler(RN_PCONTEXT pcontext, PSOCK psrc_socket, int event_code, int err_code, int nid, void* data, int size)
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
        printf("Node %i: Recieved %i bytes from Node\n", nid, size);
        break;
    case EVENT_NETWORK_CONNECTION_TX:
        printf("Node %i: Sent %i bytes to Remote Node\n", nid, size);
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


int main(int argc, char **argv)
{
    printf("Client Test\n");

    rn_initialize();
    
    rn_start_client(&context_client, SERVER_ADDRESS, SERVER_PORT, CLIENT_NODE_ID, event_handler);

    system("PAUSE");
    rn_write_data(&context_client.primary_socket,CLIENT_NODE_ID, "ABC",0,3);
    system("PAUSE");
    
    
    rn_stop_client(&context_client);
    rn_deinitialize();

    return 0;
}