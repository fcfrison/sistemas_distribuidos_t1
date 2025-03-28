#include <semaphore.h>
#include "../include/chan.h"
#include "../include/p2plink.h"
#define HANDLE_NULL_ALLOC(ptr, cleanup_label) \
    if (!(ptr)) { \
        goto cleanup_label; \
    }
#define CLEANUP(cleanup_label) \
    goto cleanup_label; 

typedef enum{
    noMX   = 0,
    wantMX = 1,
    inMX   = 2
} State;

typedef enum{
    ENTER = 0,
    EXIT  = 1
} dmxReq;

typedef struct{
}dmxResp;

typedef struct{
    chan*         Req;
    chan*         Ind;
    char**        addresses;
    unsigned char addresses_len;
    unsigned int id;
    State        st;
    char**       waiting;
    int          lcl;
    int          req_ts;
    int          nbr_resps;
    PP2PLink*    p2p;
    sem_t*       global_mutex;
}DIMEX_Module;
DIMEX_Module* new_dimex(char** addresses, unsigned char addresses_len, int id);
void format_req_entry_message(char** fmt_msg, char* entry_msg, char* sep, int id, int req_ts);
void format_req_exit_message(char**fmt_msg, char* src_msg, char* sep, int id );

