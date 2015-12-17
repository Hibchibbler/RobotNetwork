#ifndef RobotNetwork_h_
#define RobotNetwork_h_
#include <WinSock.h>
#include <Windows.h>

#include "list.h"
#include "ring_buffer.h"


#define ERR_SUCCESS                                 0  //This is a good thing.
#define ERR_NOT_IMPLEMENTED                        -1
#define ERR_FAILEDTOCREATESOCKET                   -2
#define ERR_FAILEDTOBINDSOCKET                     -3
#define ERR_FAILEDTOLISTENSOCKET                   -4
#define ERR_FAILEDTOCONNECTSOCKET                  -5
#define ERR_FAILEDTOACCEPTSOCKET                   -6
#define ERR_FAILEDTOWRITESOCKET                    -7
#define ERR_FAILEDTOREADSOCKET                     -8
#define ERR_FAILEDTOALLOCATEMEMORY                 -9

#define FLAG_NETWORK_NOT_DONE                       0
#define FLAG_NETWORK_DONE                           1
#define FLAG_NETWORK_MODE_SERVER                    0
#define FLAG_NETWORK_MODE_CLIENT                    1

#define EVENT_NETWORK_CONNECTION_NEW                0
#define EVENT_NETWORK_CONNECTION_CLOSED             1
#define EVENT_NETWORK_CONNECTION_RX                 2
#define EVENT_NETWORK_CONNECTION_TX                 3
#define EVENT_NETWORK_CONNECTION_TX_RDY             4
#define EVENT_NETWORK_ERR                           5

#define STATE_READ_ID                   0   //We are waiting for enough bytes to identify ID
#define STATE_READ_SIZE                 1   //We are waiting for enough bytes to identify payload Size
#define STATE_READ_PAYLOAD              2   //we are waiting for enough bytes to completely read the payload.

#define NETWORK_READ_MAX                2047

struct _RN_CONTEXT;
typedef struct _RN_CONTEXT RN_CONTEXT, *RN_PCONTEXT;

struct _SOCK;
typedef struct _SOCK SOCK, *PSOCK;


typedef void*(*EVENT_HANDLER)(RN_PCONTEXT pcontext, PSOCK psrc_socket, int event_code, int error, int nid, void* data, int size);

struct _SOCK{
    int             descriptor;
    char            ip[16];
    int             port;
    struct 
     sockaddr_in    socket_addr;
    
    RING_BUFFER     read_buffer;
    RING_BUFFER     write_buffer;
    int             remote_nid;
    HANDLE          read_mutex;
    HANDLE          write_mutex;

};

struct _RN_CONTEXT{
    int             nid;
    EVENT_HANDLER   event_handler;
    HANDLE          event_thread_handle;
    int             event_thread_id;
    int             is_done;
    int             network_mode;

    LIST            child_sockets;
    SOCK            primary_socket;
};



//General Interface 
int         rn_initialize             ();
int         rn_deinitialize           ();
int         rn_write_data             (PSOCK psocket,int nid, unsigned char* out_buffer, int offset, int len);

//Server Interface 
int         rn_start_server           (RN_PCONTEXT pcontext, char* ip, int port, EVENT_HANDLER event_handler);
int         rn_stop_server            (RN_PCONTEXT pcontext);

//Client Interface 
int         rn_start_client           (RN_PCONTEXT pcontext, char* ip, int port, int nid, EVENT_HANDLER event_handler);
int         rn_stop_client            (RN_PCONTEXT pcontext);

//Socket Level Interface 
int         rn_create_socket          (PSOCK psocket);
int         rn_connect_socket         (PSOCK psocket);
int         rn_bind_socket            (PSOCK psocket);
int         rn_listen_socket          (PSOCK psocket);
int         rn_accept_socket          (PSOCK psocket, PSOCK pnew_sock);
int         rn_disconnect_socket      (PSOCK psocket);
int         rn_can_read_socket        (PSOCK psocket);
int         rn_can_write_socket       (PSOCK psocket);
int         rn_read_socket            (PSOCK psocket, unsigned char* in_buffer, int offset, int len);
int         rn_write_socket           (PSOCK psocket, unsigned char* out_buffer, int offset, int len);


//Internals 
int         rn_add_new_connection     (RN_PCONTEXT pcontext);
void*       rn_server_thread          (RN_PCONTEXT pcontext);
void*       rn_client_thread          (RN_PCONTEXT pcontext);

#endif
