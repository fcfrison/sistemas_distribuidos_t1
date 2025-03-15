#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include "../include/chan.h"
#include "../include/simple_map.h"
#include "../include/errors.h"

typedef struct{
    char* to;
    char* message;
}PP2PLink_Req_Message;

typedef struct{
    char* from;
    char* message;
}PP2PLink_Ind_Message;

typedef struct{
    chan* ind;
    chan* req;
    SimpleMap* map;
}PP2PLink;
PP2PLink*
new_p2p_link(unsigned int max_size){
    chan* ind = start_chan(max_size, (void**)calloc(max_size, sizeof(PP2PLink_Ind_Message*)));
    if(!ind) return NULL;
    chan* req = start_chan(max_size, (void**)calloc(max_size, sizeof(PP2PLink_Req_Message*)));
    if(!req) return NULL;
    SimpleMap* map = create_simple_map();
    PP2PLink* p2p = (PP2PLink*)calloc(1,sizeof(PP2PLink));
    if(!p2p) return NULL;
    p2p->ind = ind;
    p2p->req = req;
    p2p->map = map;
    //start();
    return p2p;
};
int
get_server_sock(char* service, int maxpending) {
    if(maxpending<1){
        exit(EXIT_FAILURE);
    };
    struct addrinfo addr_config;
    memset(&addr_config, 0, sizeof(addr_config));
    addr_config.ai_family   = AF_INET6;
    addr_config.ai_flags    = AI_PASSIVE;
    addr_config.ai_socktype = SOCK_STREAM;
    addr_config.ai_protocol = IPPROTO_TCP;
    struct addrinfo* addr_list;
    int rtn_value = getaddrinfo(NULL, service, &addr_config, &addr_list);
    if (rtn_value != 0) {
        return -1;
    };
    int fd = -1;
    for (struct addrinfo* l_item = addr_list; l_item != NULL; l_item = l_item->ai_next) {
        fd = socket(l_item->ai_family, l_item->ai_socktype, l_item->ai_protocol);
        if (fd < 0) {
            continue;
        };
        int bind_result = bind(fd, l_item->ai_addr, l_item->ai_addrlen);
        int listen_result = listen(fd, maxpending);
        if ((bind_result == 0) && (listen_result == 0)) {
            int val = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // Enable the option
            break;
        };
    };
    return fd;
}
int
accept_client(int server_fd){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    return accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
};

typedef struct{
    int fd;
    PP2PLink* p2p;
}ListenSockArgs;


int
from_str_to_int(char* str){
    char* endptr;
    errno = 0;
    long value = strtol(str, &endptr, 10);
    if(*endptr!='\0'){
        return -1;
    };
    if(errno!=0) return -1;
    return (int)value;
};
void get_peer_info_ipv4(const int sockfd, char** ip_str, char** port_str){
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    memset(&peer_addr, 0, sizeof(peer_addr));
    if (getpeername(sockfd, (struct sockaddr*)&peer_addr, &addr_len) == 0) {
        *ip_str = (char*)calloc(INET_ADDRSTRLEN,sizeof(char));
        *port_str = (char*)calloc(6,sizeof(char));
        inet_ntop(AF_INET, &(peer_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
        int port = ntohs(peer_addr.sin_port);
        sprintf(port_str, "%d", port);
    } else {
        perror("getpeername failed");
    };
    return;
}
void
listen_to_clnt(void* args){
    ListenSockArgs* _args = (ListenSockArgs*) args;
    if(!_args||_args->fd<0 || !_args->p2p) return;
    int fd = _args->fd;
    PP2PLink* p2p = _args->p2p;
    unsigned char n_bytes_msg_l = 4;
    char msg_len_buf[n_bytes_msg_l+1];
    ssize_t rtn;
    char* ip_str = NULL, *port_str = NULL;

    while(1){
        memset(msg_len_buf,0,n_bytes_msg_l+1);
        rtn = recv(fd, msg_len_buf, n_bytes_msg_l, MSG_WAITALL);
        if(rtn<0){
            perror("Error: ");
            free(_args);
            close(fd);
            return;
        };
        int msg_len = from_str_to_int(msg_len_buf);
        if(msg_len<1){
            fprintf(stderr,"Error: invalid message length.");
            return;
        };
        char* buf_msg = (char*)calloc(msg_len+1,sizeof(char));
        rtn = recv(fd, buf_msg, msg_len,MSG_WAITALL);
        if(rtn<0){
            free(buf_msg);
            free(_args);
            close(fd);
            perror("Error: ");
            return;
        };
        if(rtn<msg_len){
            free(buf_msg);
            free(_args);
            close(fd);
            fprintf(stderr,"Error: received less bytes than expected.");
            return;
        };
        get_peer_info_ipv4(_args->fd, &ip_str, &port_str);
        if(!ip_str||!port_str){
            free(buf_msg);
            if(ip_str){
                free(ip_str);
            };
            if(port_str){
                free(port_str);
            };
            free(_args);
            close(fd);
            return;
        };
        char* from = format_rmt_add(ip_str, port_str);
        PP2PLink_Ind_Message* msg = init_p2p_ind(from, buf_msg);
        send_data(p2p->ind,(void*)msg);
        ip_str = NULL;
        port_str = NULL;
    };
};
PP2PLink_Ind_Message*
init_p2p_ind(char* from, char* message){
    PP2PLink_Ind_Message* p2p_ind = calloc(1,sizeof(PP2PLink_Ind_Message));
    if(!p2p_ind) return NULL;
    p2p_ind->from = from;
    p2p_ind->message = message;
    return p2p_ind;
}

char*
format_rmt_add(const char* ip_str, const* port_str){
    int len = strlen(ip_str) + strlen(port_str) + 2;
    char* fmt_str = (char*)calloc(len,sizeof(char));
    snprintf(fmt_str,len,"%s:%s",ip_str,port_str);
    return fmt_str;
}
void
start(PP2PLink* p2p, char* address){
    int server_fd = get_server_sock("6479", 50);
    if(server_fd==-1){
        fprintf(stderr,"Error: listening socket couldn't be created");
        exit(ERROR_FATAL);
    };
    while(1){
        int client_fd = accept_client(server_fd);
        if(client_fd==-1){
            continue;
        };
        pthread_t* conn_thread = (pthread_t*)calloc(1,sizeof(pthread_t));
        if(!conn_thread){
            close(client_fd);
            continue;
        };
        ListenSockArgs* args = (ListenSockArgs*)calloc(1,sizeof(ListenSockArgs));
        if(!args){
            free(conn_thread);
            close(client_fd);
            continue;
        };
        args->fd = client_fd;
        args->p2p = p2p;
        if(pthread_create(conn_thread,NULL,listen_to_clnt,(void*)args)!=0){
            free(args);
            free(conn_thread);
            close(client_fd);
            fprintf(stderr, "Error: failed to create thread\n");
            continue;
        };
        // a thread criada nao terá join
        pthread_detach(*conn_thread);
        free(conn_thread);
    }
}
