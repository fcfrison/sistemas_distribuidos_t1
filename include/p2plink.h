#ifndef P2PLINK_h
#define P2PLINK_h
#include "chan.h"
#include "simple_map.h"

typedef struct{
    chan* ind;
    chan* req;
    SimpleMap* map;
}PP2PLink;
typedef struct{
    int fd;
    PP2PLink* p2p;
}ListenSockArgs;

typedef struct{
    char* to;
    char* message;
}PP2PLink_Req_Message;

typedef struct{
    char* from;
    char* message;
}PP2PLink_Ind_Message;

PP2PLink* new_p2p_link(unsigned int max_size);
int get_server_sock(char* service, int maxpending);
int accept_client(int server_fd);
int from_str_to_int(char* str);
char get_peer_info_ipv4(const int sockfd, char** ip_str, char** port_str);
void* listen_to_clnt(void* args);
PP2PLink_Ind_Message* init_p2p_ind(char* from, char* message);
char* format_rmt_add(const char* ip_str, const char* port_str);
void start(PP2PLink* p2p, char* address);
#endif