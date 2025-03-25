#include "../include/chan.h"
#include "../include/p2plink.h"
#include "../include/DIMEX.h"

DIMEX_Module*
new_dimex(char** addresses, unsigned char addresses_len, int id){
    char* address = addresses[id];
    //extract port my number
    PP2PLink* p2p = new_p2p_link(1,"9000");
    chan* Req = start_chan(1, (void**)calloc(1, sizeof(dmxReq*)));
    chan* Ind = start_chan(1, (void**)calloc(1, sizeof(dmxResp*)));
    char** waiting = (char**)calloc(addresses_len,sizeof(char*));
    for (size_t i = 0; i < addresses_len; i++){
        addresses[i] = calloc(1,sizeof(char));
        *addresses[i] = 0;
    };
    DIMEX_Module* dimex = (DIMEX_Module*)calloc(1,sizeof(DIMEX_Module));
    dimex->Req = Req;
    dimex->Ind = Ind;
    dimex->addresses = addresses;
    dimex->addresses_len = addresses_len;
    dimex->id = id;
    dimex->st = noMX;
    dimex->lcl = 0;
    dimex->req_ts = 0;
    dimex->nbr_resps = 0;
    dimex->waiting = waiting;
    dimex->p2p = p2p;
    return dimex;
};

void
start_dimex(DIMEX_Module* module){
    // Create two threads, on to handle messages from the app
    // another to handle messages coming from other processes
}

