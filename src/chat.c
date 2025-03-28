#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/errors.h"
#include "../include/p2plink.h"
#define MAX_LEN 256
void*
receiver(void* _p2p){
    PP2PLink* p2p = (PP2PLink*)_p2p;
     while(1){
         PP2PLink_Ind_Message* msg = receive_data(p2p->ind);
         fprintf(stdout, "Message from: %s\n", msg->from);
         fprintf(stdout, "Message content: %s\n", msg->message);
         if(msg && msg->from) free(msg->from);
         if(msg && msg->message) free(msg->message);
         if(msg) free(msg);
     };
     return NULL;
 }
 
 int __main(void){
     char* address = "8051";
     char* server = calloc(strlen(address),sizeof(char));
     strcpy(server,address);
     PP2PLink* p2p = new_p2p_link(1,server);
     pthread_t* receiver_th = (pthread_t*)calloc(1,sizeof(pthread_t));
     if(pthread_create(receiver_th, NULL, receiver, (void*)p2p)!=0){
        free(receiver_th);
        fprintf(stderr, "Error: failed to create thread\n");
        exit(ERROR_FATAL);
    };
    char* to = "127.0.0.1:9000";
    while(1){
        char msg[MAX_LEN];
        memset(msg,0,MAX_LEN);
        printf("Type a message: \n");
        fgets(msg, MAX_LEN, stdin);
        size_t msg_len = strlen(msg)+1;
        char* __msg = (char*)calloc(msg_len,sizeof(char));
        char* __to = (char*)calloc(strlen(to)+1,sizeof(char));
        strcpy(__msg, msg);
        strcpy(__to,to);
        PP2PLink_Req_Message* req = calloc(1,sizeof(PP2PLink_Req_Message));
        req->message = __msg;
        req->to      = __to;
        send_data(p2p->req,req);
    }

 }