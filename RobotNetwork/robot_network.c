#include "robot_network.h"
#include <stdio.h>

////////////////////////////////
int rn_initialize()
{
    //If windows
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(1,1), &wsa_data );
    //Endif
    return ERR_SUCCESS;
}

int rn_deinitialize()
{
    //If windows
    WSACleanup();
    //Endif
    return ERR_SUCCESS;
}
////////////////////////////////
int rn_write_data(PSOCK psocket,int nid, unsigned char* out_buffer, int offset, int len)
{
    WaitForSingleObject(psocket->write_mutex,INFINITE);
        rb_add_data(&psocket->write_buffer, (DATUM*)&nid, 4);
        rb_add_data(&psocket->write_buffer, (DATUM*)&len, 4);
        rb_add_data(&psocket->write_buffer,out_buffer+offset,len);
    ReleaseMutex(psocket->write_mutex);
    return 0;
}
////////////////////////////////
int rn_start_server(RN_PCONTEXT pcontext, char* ip, int port, EVENT_HANDLER event_handler)
{

    //Initialize context for listening socket 
    memcpy(pcontext->primary_socket.ip , ip, 16);
    pcontext->primary_socket.port = port;
    
    pcontext->event_handler = event_handler;

    list_initialize(&pcontext->child_sockets);

    rn_create_socket(&pcontext->primary_socket);
    rn_bind_socket  (&pcontext->primary_socket);
    rn_listen_socket(&pcontext->primary_socket);


    pcontext->network_mode = FLAG_NETWORK_MODE_SERVER;
    pcontext->is_done = FLAG_NETWORK_NOT_DONE;
    pcontext->event_thread_handle = CreateThread(NULL,
                                                 0,
                                                 (LPTHREAD_START_ROUTINE)rn_server_thread, 
                                                 pcontext, 
                                                 0, 
                                                 (DWORD*)&pcontext->event_thread_id);
    return ERR_SUCCESS;
}

int rn_stop_server(RN_PCONTEXT pcontext)
{
    pcontext->is_done = FLAG_NETWORK_DONE;
    WaitForSingleObject(pcontext->event_thread_handle,INFINITE);
    closesocket(pcontext->primary_socket.descriptor);
    
    CloseHandle(pcontext->primary_socket.read_mutex);
    CloseHandle(pcontext->primary_socket.write_mutex);
    return ERR_SUCCESS;
}
////////////////////////////////
int rn_start_client (RN_PCONTEXT pcontext, char* ip, int port, int nid, EVENT_HANDLER event_handler)
{
    //Initialize context for outgoing socket
    memcpy(pcontext->primary_socket.ip , ip, 16);
    pcontext->primary_socket.port = port;
    pcontext->nid = nid;
    pcontext->event_handler = event_handler;

    //Create Socket
    rn_create_socket(&pcontext->primary_socket);

    pcontext->primary_socket.socket_addr.sin_family  = AF_INET;
    pcontext->primary_socket.socket_addr.sin_port    = htons(pcontext->primary_socket.port);
    pcontext->primary_socket.socket_addr.sin_addr.s_addr = inet_addr(pcontext->primary_socket.ip);

    pcontext->primary_socket.read_mutex = CreateMutex(NULL,0,NULL);
    pcontext->primary_socket.write_mutex = CreateMutex(NULL,0,NULL);

    rb_initialize(&pcontext->primary_socket.read_buffer, 16777216);
    rb_initialize(&pcontext->primary_socket.write_buffer, 16777216);
    
    //Start event pump
    pcontext->network_mode = FLAG_NETWORK_MODE_CLIENT;
    pcontext->is_done = FLAG_NETWORK_NOT_DONE;
    pcontext->event_thread_handle = CreateThread(NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)rn_client_thread, 
                                                pcontext, 
                                                0, 
                                                (DWORD*)&pcontext->event_thread_id);

    return ERR_SUCCESS;
}

int rn_stop_client (RN_PCONTEXT pcontext)
{
    pcontext->is_done = FLAG_NETWORK_DONE;
    WaitForSingleObject(pcontext->event_thread_handle,INFINITE);
    closesocket(pcontext->primary_socket.descriptor);

    CloseHandle(pcontext->primary_socket.read_mutex);
    CloseHandle(pcontext->primary_socket.write_mutex);
    
    return ERR_SUCCESS;
}
////////////////////////////////
int rn_create_socket(PSOCK psocket)
{
    //Create
    psocket->descriptor = (int)socket(PF_INET,SOCK_STREAM,0);
    if (psocket->descriptor < 0)
    {
        printf("rn_create_socket() Failed.\n");
        return ERR_FAILEDTOCREATESOCKET;
    }
    return ERR_SUCCESS;
}

int rn_bind_socket(PSOCK psocket)
{
    //Bind
    int iOptVal=1;

    psocket->socket_addr.sin_family = AF_INET;
    psocket->socket_addr.sin_port = htons(psocket->port);
    psocket->socket_addr.sin_addr.s_addr = inet_addr(psocket->ip);

    //  Reuse address if it is already in use. mostly usefuly for debugging.
    setsockopt(psocket->descriptor,SOL_SOCKET,SO_REUSEADDR,(char*)&iOptVal,sizeof(int));

    if (bind(psocket->descriptor,(struct sockaddr*)&psocket->socket_addr,sizeof(struct sockaddr)) < 0)
    {
        printf("rn_listen_socket() Failed to bind.\n");
        return ERR_FAILEDTOBINDSOCKET;
    }
    return ERR_SUCCESS;
}

int rn_listen_socket (PSOCK psocket)
{    
    //Listen
    if (listen(psocket->descriptor,100))
    {
        printf("rn_listen_socket() Failed to listen.\n");
        return ERR_FAILEDTOLISTENSOCKET;
    }

    //We're good.
    return ERR_SUCCESS;
}

int rn_accept_socket(PSOCK plistener_socket, PSOCK pnew_socket)
{
    int len = sizeof(struct sockaddr_in);

    pnew_socket->descriptor = accept(plistener_socket->descriptor ,(struct sockaddr*)&pnew_socket->socket_addr,&len);
    if (pnew_socket->descriptor == INVALID_SOCKET){
        printf("rn_accept_socket() Failed to accept incoming connection.\n");
        return ERR_FAILEDTOACCEPTSOCKET;
    }
    return ERR_SUCCESS;
}

int rn_connect_socket(PSOCK psocket)
{
    if (connect(psocket->descriptor, (struct sockaddr*)&psocket->socket_addr, sizeof(struct sockaddr_in)))
    {    
        //printf("rn_connect_socket() Failed to connect.\n");
        return ERR_FAILEDTOCONNECTSOCKET;
    }
    return ERR_SUCCESS;
}

int rn_disconnect_socket(PSOCK psocket)
{
    return ERR_SUCCESS;
}

int rn_can_read_socket(PSOCK psocket)
{
    struct timeval tv;
    fd_set rdset;

    tv.tv_sec = 0;
    tv.tv_usec= 5000;
    
    FD_ZERO(&rdset);
    FD_SET(psocket->descriptor, &rdset);
    if (select(psocket->descriptor+1,&rdset,0,0,&tv))
        return 1;
    return 0;
}

int rn_can_write_socket(PSOCK psocket)
{
    struct timeval tv;
    fd_set wrset;

    tv.tv_sec = 0;
    tv.tv_usec= 5000;
    
    FD_ZERO(&wrset);
    FD_SET(psocket->descriptor,&wrset);
    if (select(psocket->descriptor+1,0,&wrset,0,&tv))
        return 1;
    return 0;
}

int rn_read_socket (PSOCK psocket, unsigned char* in_buffer, int offset, int len)
{
    int read = recv(psocket->descriptor ,((char*)in_buffer)+offset, len,0);
    if (read == SOCKET_ERROR)
    {
        if ( WSAGetLastError() == WSAECONNRESET)
        {
            printf("Remote Host Abruptly Disconnected.\n");
            return 0;
        }else
        {
            printf("rn_read_socket => recv() failed: %i.\n", WSAGetLastError());
        }
    }
    return read;
}

int rn_write_socket(PSOCK psocket, unsigned char* out_buffer, int offset, int len)
{
    int written = send(psocket->descriptor,((char*)out_buffer)+offset,len,0);
    //if (written == SOCKET_ERROR)
    //    printf("rn_write_socket() => send() failed: %i\n", WSAGetLastError());
    return written;
}
////////////////////////////////
void* rn_server_thread(RN_PCONTEXT pcontext)
{
    PLIST_NODE cur_child_socket;
    int ret;
    int cur_index;
    int bytes_read;
    int bytes_written;
    int data_packet_id;
    int data_packet_len;
    unsigned char* temp_buffer;
    SOCK new_socket;

    temp_buffer = (unsigned char*)malloc(16777216);
    if (temp_buffer == NULL)
    {
        printf("rn_server_thread() => Unable to allocate memory for temporary io buffer. Server Thread Stopping.\n");
        pcontext->event_handler(pcontext, NULL, EVENT_NETWORK_ERR, ERR_FAILEDTOALLOCATEMEMORY, 0, NULL, 0);
        return NULL;
    }
    
    while (pcontext->is_done == FLAG_NETWORK_NOT_DONE)
    {
        if (pcontext->network_mode == FLAG_NETWORK_MODE_SERVER)
        {
            //Service Pending Accepts on listening socket
            if (rn_can_read_socket(&pcontext->primary_socket))
            {
               
                rn_accept_socket(&pcontext->primary_socket,&new_socket);

                new_socket.remote_nid = -1;

                rb_initialize(&new_socket.write_buffer, 16777216);
                rb_initialize(&new_socket.read_buffer, 16777216);
                
                new_socket.read_mutex = CreateMutex(NULL,0,NULL);
                new_socket.write_mutex = CreateMutex(NULL,0,NULL);

                list_add_node(&pcontext->child_sockets, &new_socket,sizeof(SOCK),MODE_BEG);
                //pcontext->event_handler( pcontext, &new_socket, EVENT_NETWORK_CONNECTION_NEW, 0, NULL, 0);
            }else
            {

            }

            //Service Pending Writes
            cur_child_socket = pcontext->child_sockets.head;
            cur_index = 0;

            for(;cur_child_socket != NULL;cur_child_socket = cur_child_socket->next, cur_index++)
            {
                PSOCK child_socket = (PSOCK)cur_child_socket->data;
                if (rn_can_write_socket(child_socket))
                {
                    pcontext->event_handler(pcontext, child_socket, EVENT_NETWORK_CONNECTION_TX_RDY, ERR_SUCCESS, child_socket->remote_nid, NULL, 0);
                    WaitForSingleObject(child_socket->write_mutex,INFINITE);
                        ret = rb_get_used(&child_socket->write_buffer);
                        if (ret > 0)
                        {
                            rb_get_data(&child_socket->write_buffer,temp_buffer,ret);
                            bytes_written = rn_write_socket(child_socket,temp_buffer,0,ret);
                            if (bytes_written >= 0)
                            {
                                rb_remove_data(&child_socket->write_buffer,bytes_written);
                            }
                            else if (bytes_written < 0)
                            {
                                //Write failed
                                printf("rn_server_thread() => rn_write_socket() failed.\n");
                                pcontext->event_handler(pcontext, child_socket,EVENT_NETWORK_ERR, ERR_FAILEDTOWRITESOCKET, child_socket->remote_nid, NULL, 0);
                                return NULL;
                            }
                        }
                    ReleaseMutex(child_socket->write_mutex);
                }
            }
            //Service Pending Reads on client sockets
reset_loop:
            cur_child_socket = pcontext->child_sockets.head;
            cur_index = 0;

        
            for(;cur_child_socket != NULL;cur_child_socket = cur_child_socket->next, cur_index++)
            {
                PSOCK child_socket = (PSOCK)cur_child_socket->data;
                if (rn_can_read_socket(child_socket))
                {
                    bytes_read = rn_read_socket(child_socket, temp_buffer, 0, NETWORK_READ_MAX);
                    if (bytes_read == 0)
                    {
                        pcontext->event_handler(pcontext, child_socket, EVENT_NETWORK_CONNECTION_CLOSED, ERR_SUCCESS, child_socket->remote_nid, 0, 0);
                        rb_deinitialize(&child_socket->read_buffer);
                        rb_deinitialize(&child_socket->write_buffer);
                        list_del_node_at(&pcontext->child_sockets, cur_index);
                        
                        //Reset current child context pointer to beginning, because 
                        // we altered the list by removing a remotely closed client.
                        cur_child_socket = pcontext->child_sockets.head;
                        cur_index = 0;
                        goto reset_loop;

                    }
                    else if (bytes_read > 0)
                    {
                        //we got some good data...
                        //buffer it.
                        rb_add_data(&child_socket->read_buffer, temp_buffer, bytes_read);
                    }
                    else if (bytes_read < 0)
                    {
                        printf("rn_server_thread() => rn_read_socket() failed.\n");
                        pcontext->event_handler(pcontext, child_socket,EVENT_NETWORK_ERR, ERR_FAILEDTOREADSOCKET, child_socket->remote_nid, NULL, 0);
                        return NULL;
                    }

                    if (rb_get_used(&child_socket->read_buffer) >= 8)
                    {

                        //We have enough to examine header.
                        rb_get_data(&child_socket->read_buffer,temp_buffer, 8);
                        data_packet_id  = *((unsigned int*)temp_buffer);
                        data_packet_len  = *((unsigned int*)(temp_buffer+4));

                        if (rb_get_used(&child_socket->read_buffer)-8 >= data_packet_len)
                        {
                            //we have all the bytes of the payload
                            // deliver to user.
                            //remove entire packet from ring buffer.
                            rb_remove_data(&child_socket->read_buffer,8);
                            rb_get_data(&child_socket->read_buffer,temp_buffer,data_packet_len);
                            rb_remove_data(&child_socket->read_buffer,data_packet_len);


                            //If we have not yet recieved a node id, than, this packet
                            //indicates the master node id, and the payload is ignored and discarded.
                            if (child_socket->remote_nid == -1)
                            {
                                child_socket->remote_nid = data_packet_id;
                                pcontext->event_handler(pcontext, child_socket,EVENT_NETWORK_CONNECTION_NEW, ERR_SUCCESS, child_socket->remote_nid, NULL, 0);
                            }else
                            {
                                pcontext->event_handler(pcontext, child_socket,EVENT_NETWORK_CONNECTION_RX, ERR_SUCCESS, child_socket->remote_nid, temp_buffer, data_packet_len);
                            }
                        }else
                        {
                            //otherwise we do not yet have the entire packet, so
                            //keep the partial data in the ring buffer until then.
                            // perhaps next read...
                            //printf ("[%i]",data_packet_id);//debug
                        }
                    }
                }
            }
        }
    }
    free(temp_buffer);
    return NULL;
}

void* rn_client_thread(RN_PCONTEXT pcontext)
{
    int ret;
    int bytes_read;
    int bytes_written;
    int data_packet_id;
    int data_packet_len;
    unsigned char* temp_buffer;
    PSOCK client_socket;

    temp_buffer = (unsigned char*)malloc(16777216);
    if (temp_buffer == NULL)
    {
        printf("rn_client_thread() => Unable to allocate memory for temporary io buffer\n");
        pcontext->event_handler(pcontext, NULL,EVENT_NETWORK_ERR, ERR_FAILEDTOALLOCATEMEMORY, 0, NULL, 0);
        return NULL;
    }

    client_socket = (PSOCK)&pcontext->primary_socket;

     //Connect Socket
    ret = rn_connect_socket(client_socket);
    if (ret == ERR_FAILEDTOCONNECTSOCKET)
    {
        printf("Unable To Connect to Remote Host\n");
        pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_ERR, ERR_FAILEDTOCONNECTSOCKET, pcontext->nid, NULL, 0);
        return NULL;
    }
    
    //This is where we send the initial node id to the server, which gets us fully connected.
    rn_write_data(client_socket, pcontext->nid, NULL,0,0);

    pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_CONNECTION_NEW, ERR_SUCCESS, pcontext->nid, NULL, 0);
    while (pcontext->is_done == FLAG_NETWORK_NOT_DONE)
    {
        //Service Pending Writes
        if (rn_can_write_socket(client_socket))
        {
            WaitForSingleObject(client_socket->write_mutex,INFINITE);
                ret = rb_get_used(&client_socket->write_buffer);
                if (ret > 0)
                {
                    rb_get_data(&client_socket->write_buffer,temp_buffer,ret);
                    bytes_written = rn_write_socket(client_socket,temp_buffer,0,ret);
                    if (bytes_written >= 0)
                    {
                        rb_remove_data(&client_socket->write_buffer,bytes_written);
                        //pcontext->event_handler(pcontext, client_socket, EVENT_NETWORK_CONNECTION_TX_RDY, ERR_SUCCESS, client_socket->remote_nid, NULL, 0);
                        pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_CONNECTION_TX, ERR_SUCCESS, pcontext->nid, NULL, 0);
                    }
                    else if (bytes_written < 0)
                    {
                        //Write failed
                        printf("rn_client_thread() => rn_write_socket() failed.\n");
                        pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_ERR, ERR_FAILEDTOWRITESOCKET, pcontext->nid, NULL, 0);
                        return NULL;
                    }
                }
            ReleaseMutex(client_socket->write_mutex);
        }
            
        //Service Pending Reads on client sockets
        if (rn_can_read_socket(client_socket))
        {
                    
            bytes_read = rn_read_socket(client_socket, temp_buffer, 0, NETWORK_READ_MAX);
            if (bytes_read == 0)
            {
                pcontext->event_handler(pcontext, client_socket, EVENT_NETWORK_CONNECTION_CLOSED, ERR_SUCCESS, pcontext->nid, NULL, 0);
                rb_deinitialize(&client_socket->read_buffer);
                rb_deinitialize(&client_socket->write_buffer);
                break;

            }
            else if (bytes_read > 0)
            {
                //we got some good data...
                //buffer it.
                rb_add_data(&client_socket->read_buffer, temp_buffer, bytes_read);
            }
            else if (bytes_read < 0)
            {
                printf("rn_client_thread() => rn_read_socket() failed.\n");
                pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_ERR, ERR_FAILEDTOREADSOCKET, pcontext->nid, NULL, 0);
                return NULL;
            }

            if (rb_get_used(&client_socket->read_buffer) >= 8)
            {
                rb_get_data(&client_socket->read_buffer,temp_buffer, 8);
                data_packet_id  = *((unsigned int*)temp_buffer);
                data_packet_len  = *((unsigned int*)(temp_buffer+4));

                if (rb_get_used(&client_socket->read_buffer)-8 >= data_packet_len)
                {
                    //We have all bytes of the payload...deliver to user.
                    //remove entire packet from ring buffer.
                    rb_remove_data(&client_socket->read_buffer,8);
                    rb_get_data(&client_socket->read_buffer,temp_buffer,data_packet_len);
                    rb_remove_data(&client_socket->read_buffer,data_packet_len);
                    pcontext->event_handler(pcontext, client_socket,EVENT_NETWORK_CONNECTION_RX, ERR_SUCCESS, data_packet_id, temp_buffer, data_packet_len);
                }else
                {
                    //otherwise we do not yet have the entire packet, so
                    //keep the partial data in the ring buffer until then.
                    // perhaps next read...
                    //printf ("[%i]",data_packet_id);//debug
                }
            }
        }
    }
    free(temp_buffer);
    return NULL;
}




