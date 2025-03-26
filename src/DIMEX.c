#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/chan.h"
#include "../include/p2plink.h"
#include "../include/DIMEX.h"

DIMEX_Module*
__new_dimex(char** addresses, unsigned char addresses_len, int id){
    char* address = addresses[id];
    //extract port my number
    //TODO: probably it's going to be necessary to extract the port from 
    // the address in the form 127.0.0.1:9000
    PP2PLink* p2p = new_p2p_link(1,"9000");
    chan*     Req = start_chan(1, (void**)calloc(1, sizeof(dmxReq*)));
    chan*     Ind = start_chan(1, (void**)calloc(1, sizeof(dmxResp*)));
    char** waiting = (char**)calloc(addresses_len,sizeof(char*));
    for (size_t i = 0; i < addresses_len; i++){
        waiting[i] = calloc(1,sizeof(char));
        *waiting[i] = 0;
    };
    DIMEX_Module* dimex = (DIMEX_Module*)calloc(1,sizeof(DIMEX_Module));
    dimex->Req       = Req;
    dimex->Ind       = Ind;
    dimex->addresses = addresses;
    dimex->addresses_len = addresses_len;
    dimex->id        = id;
    dimex->st        = noMX;
    dimex->lcl       = 0;
    dimex->req_ts    = 0;
    dimex->nbr_resps = 0;
    dimex->waiting   = waiting;
    dimex->p2p       = p2p;
    return dimex;
};
DIMEX_Module*
new_dimex(char** addresses, unsigned char addresses_len, int id){
    DIMEX_Module* dimex =__new_dimex(addresses, addresses_len, id);
    start_dimex(dimex);
    return dimex;
}
void*
handle_msg_from_app(DIMEX_Module* module){
    while(1){
        dmxReq* dmxR = receive_data(module->Req);
        switch (*dmxR){
            case ENTER:
                printf("app request mutex - process id%d\n",module->id);
                //handleUponReqEntry()
                break;
            case EXIT:
                //handleUponReqExit() 
                break;
        
        default:
            break;
        };
        if(dmxR){
            free(dmxR);
        };
    }

}
void*
handle_peer_msg(DIMEX_Module* module){
    PP2PLink_Ind_Message* msg_outro = NULL;
while (1){
    msg_outro = receive_data(module->p2p->ind);
    if(!msg_outro){
        msg_outro = NULL;
        continue;
    };
    HANDLE_NULL_ALLOC(msg_outro->from,cleanup_msg_ptr);
    HANDLE_NULL_ALLOC(msg_outro->message,cleanup_msg_ptr);
    if(strstr(msg_outro->message,"respOK")){
        //handleUponDeliverRespOk
    }else if(strstr(msg_outro->message,"reqEntry")){
        //handleUponDeliverReqEntry
    }else{
        FREE_MEMORY(msg_outro,free_mem);
    }
    };
    cleanup_msg_ptr:
        if(msg_outro->message) free(msg_outro->message);
        if(msg_outro->from)    free(msg_outro->from);
        if(msg_outro)          free(msg_outro);
    free_mem:
        if(msg_outro->message) free(msg_outro->message);
        if(msg_outro->from)    free(msg_outro->from);
        if(msg_outro)          free(msg_outro);
}
void
start_dimex(DIMEX_Module* module){
    // Create two threads, one to handle messages from the app
    pthread_t* handle_app_th  = NULL;
    pthread_t* handle_peer_msg_th = NULL;
    handle_app_th = (pthread_t*)calloc(1,sizeof(pthread_t));
    handle_peer_msg_th = (pthread_t*)calloc(1,sizeof(pthread_t));
    HANDLE_NULL_ALLOC(handle_app_th, cleanup_calloc_error);
    HANDLE_NULL_ALLOC(handle_peer_msg_th, cleanup_calloc_error);
    if(pthread_create(handle_app_th, NULL, handle_msg_from_app, module)!=0){
        if(handle_app_th) free(handle_app_th);
        fprintf(stderr, "Error: failed to create thread\n");
        exit(1);
    };
    // another to handle messages coming from other processes
    if(pthread_create(handle_peer_msg_th, NULL, handle_peer_msg, module)!=0){
        if(handle_peer_msg_th) free(handle_peer_msg_th);
        fprintf(stderr, "Error: failed to create thread\n");
        exit(1);
    };
    cleanup_calloc_error:
        if(handle_app_th) free(handle_app_th);
        if(handle_peer_msg_th) free(handle_peer_msg_th);
        fprintf(stderr, "Error: failed to allocate memory\n");
        exit(1);
}

