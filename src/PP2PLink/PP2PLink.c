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
#include <signal.h>
#include "../include/chan.h"
#include "../include/simple_map.h"
#include "../include/errors.h"
#include "../include/p2plink.h"
#define HANDLE_NULL_ALLOC(ptr, cleanup_label) \
    if (!(ptr)) { \
        goto cleanup_label; \
    }

PP2PLink*
__new_p2p_link(unsigned int max_size){
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
    return p2p;
};
PP2PLink*
new_p2p_link(unsigned int max_size, char* address){
    PP2PLink* p2p = __new_p2p_link(max_size);
    start(p2p,address);
    return p2p;
}
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
};
int
accept_client(int server_fd){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    return accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
};

int
from_str_to_int(char* str){
    if(!str || !strlen(str)) return -1;
    char* endptr;
    errno = 0;
    long value = strtol(str, &endptr, 10);
    if(*endptr!='\0'){
        return -1;
    };
    if(errno!=0) return -1;
    return (int)value;
};
char 
get_peer_info_ipv4(const int sockfd, char** ip_str, char** port_str){
    if(sockfd<0 || !ip_str || !port_str){
        return -1;
    };
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    memset(&peer_addr, 0, sizeof(peer_addr));
    if (getpeername(sockfd, (struct sockaddr*)&peer_addr, &addr_len) == 0) {
        *ip_str = (char*)calloc(INET_ADDRSTRLEN,sizeof(char));
        if(!(*ip_str)){
            return -1;
        }
        *port_str = (char*)calloc(6,sizeof(char));
        if(!(*port_str)){
            free(*ip_str);
            return -1;
        }
        inet_ntop(AF_INET, &(peer_addr.sin_addr), *ip_str, INET_ADDRSTRLEN);
        int port = ntohs(peer_addr.sin_port);
        sprintf(*port_str, "%d", port);
    } else {
        perror("getpeername failed");
        return -1;
    };
    return 1;
};
void*
listen_to_clt(void* args){
    ListenSockArgs* _args = (ListenSockArgs*) args;
    if(!_args||_args->fd<0 || !_args->p2p) return NULL;
    int fd = _args->fd;
    PP2PLink* p2p = _args->p2p;
    unsigned char n_bytes_msg_l = 4;
    char msg_msg_size[n_bytes_msg_l+1];
    ssize_t rtn;
    char* ip_str = NULL, *port_str = NULL;
    while(1){
        memset(msg_msg_size,0,n_bytes_msg_l+1);
        rtn = recv(fd, msg_msg_size, n_bytes_msg_l, MSG_WAITALL);
        if(rtn<0){
            perror("Error: ");
            free(_args);
            close(fd);
            return NULL;
        };
        int msg_len = from_str_to_int(msg_msg_size);
        if(msg_len<1){
            close(fd);
            fprintf(stderr,"Error: invalid message length.");
            return NULL;
        };
        char* buf_msg = (char*)calloc(msg_len+1,sizeof(char));
        rtn = recv(fd, buf_msg, msg_len,MSG_WAITALL);
        printf("Message received: %s\n",buf_msg);
        if(rtn<0){
            free(buf_msg);
            free(_args);
            close(fd);
            perror("Error: ");
            return NULL;
        };
        if(rtn<msg_len){
            free(buf_msg);
            free(_args);
            close(fd);
            fprintf(stderr,"Error: received less bytes than expected.");
            return NULL;
        };
        if(get_peer_info_ipv4(_args->fd, &ip_str, &port_str)==-1){
            free(buf_msg);
            if(ip_str){
                free(ip_str);
            };
            if(port_str){
                free(port_str);
            };
            free(_args);
            close(fd);
            return NULL;
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
format_rmt_add(const char* ip_str, const char* port_str){
    if(!ip_str||!port_str) return NULL;
    int len = strlen(ip_str) + strlen(port_str) + 2;
    char* fmt_str = (char*)calloc(len,sizeof(char));
    snprintf(fmt_str,len,"%s:%s",ip_str,port_str);
    return fmt_str;
}
void
start(PP2PLink* p2p, char* address){
    ListenerArgs* l_args =calloc(1,sizeof(ListenerArgs));
    l_args->maxpending = 50;
    l_args->p2p        = p2p;
    l_args->port       = address;
    pthread_t* listener_th = (pthread_t*)calloc(1,sizeof(pthread_t));
    if(pthread_create(listener_th, NULL, Listener, l_args)!=0){
        free(listener_th);
        fprintf(stderr, "Error: failed to create thread\n");
        exit(ERROR_FATAL);
    };
    pthread_t* sender_th = (pthread_t*)calloc(1,sizeof(pthread_t));
    if(pthread_create(sender_th, NULL, Sender, p2p)!=0){
        free(sender_th);
        fprintf(stderr, "Error: failed to create thread\n");
        exit(ERROR_FATAL);
    };
    free(listener_th);
    free(sender_th);
    return;
};
void*
Sender(void* args){
    PP2PLink* p2p = (PP2PLink*)args;
    while(1){
        PP2PLink_Req_Message* req = receive_data(p2p->req);
        Send(req, p2p);
    };
}
PP2PLink_Req_Message*
init_p2p_req(char* to, char* message){
    PP2PLink_Req_Message* req = (PP2PLink_Req_Message*)calloc(1,sizeof(PP2PLink_Req_Message));
    if(!req) return NULL;
    req->message = message;
    req->to      = to;
    return req;
}
void*
Listener(void* l_args){
    ListenerArgs* lstn_args = (void*)l_args;
    PP2PLink* p2p = lstn_args->p2p;
    char* port    = lstn_args->port;
    int maxpending = lstn_args->maxpending;
    free(lstn_args);
    int server_fd = get_server_sock(port, maxpending);
    if(server_fd==-1){
        fprintf(stderr,"Error: listening socket couldn't be created");
        exit(ERROR_FATAL);
    };
    while(1){
        int client_fd = accept_client(server_fd);
        if(client_fd==-1){
            continue;
        };
        printf("Message: new connection from %d\n",client_fd);
        pthread_t* conn_thread = (pthread_t*)calloc(1,sizeof(pthread_t));
        if(!conn_thread){
            close(client_fd);
            continue;
        };
        ListenSockArgs* args = init_listen_sock_args(client_fd,p2p);
        if(!args){
            free(conn_thread);
            close(client_fd);
            continue;
        };
        if(pthread_create(conn_thread,NULL,listen_to_clt,(void*)args)!=0){
            free(args);
            free(conn_thread);
            close(client_fd);
            fprintf(stderr, "Error: failed to create thread\n");
            continue;
        };
        // a thread criada nao terá join
        pthread_detach(*conn_thread);
        free(conn_thread);
    };
}
ListenSockArgs*
init_listen_sock_args(int client_fd, PP2PLink* p2p){
    if(!p2p||client_fd<0) return NULL;
    ListenSockArgs* args = (ListenSockArgs*)calloc(1,sizeof(ListenSockArgs));
    if(!args) return NULL;
    args->fd  = client_fd;
    args->p2p = p2p;
    return args;
}


void
Send(PP2PLink_Req_Message* req, PP2PLink* p2p){
    int*  fd   = NULL;
    char* ip   = NULL; 
    char* port = NULL;
    int   socket_fd;
    KeyValuePair* kvp = NULL;
    int max_retries = 1, count_retries = -1;
    int err;
    if(!req){
        return;
    };
    char* key      = req->to;
    char* message  = req->message;
    HANDLE_NULL_ALLOC(key, cleanup_req_ip_port_kvp);
    HANDLE_NULL_ALLOC(message, cleanup_req_ip_port_kvp);
    int message_len = (int)strlen(message);
    from_key_extract_ip_port(&ip, &port, key, ":");
    HANDLE_NULL_ALLOC(ip, cleanup_req_ip_port_kvp);
    HANDLE_NULL_ALLOC(port, cleanup_req_ip_port_kvp);
    char msg_size[4] = {0};
    sprintf(msg_size, "%d", message_len);
    // Ignore SIGPIPE signal globally
    signal(SIGPIPE, SIG_IGN);
    while(count_retries<max_retries){
        kvp = get(p2p->map, key, compare_keys);
        if(!kvp){
            HANDLE_NULL_ALLOC(dial(atoi(port), ip, &fd), cleanup_req_ip_port_kvp);
            cache_connection(key, fd, &kvp,p2p->map);
        };
        socket_fd = *((int*)kvp->value);
        free(kvp);
        kvp = NULL;
        err = write(socket_fd, msg_size, 4);
        if(err<0){
            count_retries++;
            handle_write_error(socket_fd, key, p2p->map);
            continue;
        };
        err = write(socket_fd,message,message_len);
        if(err<0){
            count_retries++;
            handle_write_error(socket_fd, key, p2p->map);
            continue;
        };
        break;
    };
    HANDLE_NULL_ALLOC(NULL, cleanup_req_ip_port_kvp);
    return;

    cleanup_req_ip_port_kvp:
        if(req && req->message) free(req->message);
        if(req && req->to)      free(req->to);
        if(req)                 free(req);
        if(ip)                  free(ip);
        if(port)                free(port);
        if(kvp)                 free(kvp);
        return;

};
void
handle_write_error(int fd, const char* key, SimpleMap* map){
    close(fd);
    KeyValuePair rmv_pair;
    remove_key(map, key, compare_keys, &rmv_pair);
    free(rmv_pair.key);
    free(rmv_pair.value);
    return;
};
void
cache_connection(const char*   key,
                int*           fd,
                KeyValuePair** kvp,
                SimpleMap*     sm){
    if(!key || *fd<0 || !sm){
        fd = NULL;
        return;
    }
    char* key_cpy = (char*)calloc(strlen(key)+1,sizeof(char));
    strcpy(key_cpy,key);
    *kvp = create_key_val_pair((void*)key_cpy,fd);
    set(sm,*kvp,compare_keys);
    fd = NULL;
    return;
};
const void*
compare_keys(const void* key_a, const void* key_b){
    if(!key_a||!key_b) return NULL;
    return strcmp((char*)key_a, (char*)key_b)==0?key_a:NULL;
};
void
from_key_extract_ip_port(char** ip,
                         char** port,
                         const char* to,
                         const char* delim){
    if(!ip||!port||!to||!delim){
        *ip=NULL;
        *port=NULL;
        return;
    }
    size_t len = strlen(to);
    char* next=NULL;
    HANDLE_NULL_ALLOC(len, cleanup_none);
    char* msg_cp = (char*)calloc(len+1,sizeof(char));
    HANDLE_NULL_ALLOC(msg_cp, cleanup_none);
    strcpy(msg_cp,to);
    next = strtok(msg_cp,delim);
    HANDLE_NULL_ALLOC(next, cleanup_msg);
    *ip = (char*)calloc(strlen(next)+1,sizeof(char));
    HANDLE_NULL_ALLOC(*ip, cleanup_msg);
    strcpy(*ip,next);
    next = strtok(NULL,delim);
    HANDLE_NULL_ALLOC(next, cleanup_msg_ip);
    *port = (char*)calloc(strlen(next)+1,sizeof(char));
    HANDLE_NULL_ALLOC(*port, cleanup_msg_ip);
    strcpy(*port,next);
    free(msg_cp);
    return;

    cleanup_none:
        *ip = NULL;
        *port = NULL;
        return;
    cleanup_msg:
        if(msg_cp) free(msg_cp);
        *ip = NULL;
        *port = NULL;
        return;
    cleanup_msg_ip:
        if(msg_cp) free(msg_cp);
        if(*ip)    free(*ip);
        *ip = NULL;
        *port = NULL;
        return;
};
unsigned char
dial(const int port, const char* ip, int** sockfd){
    *sockfd = (int*)calloc(1,sizeof(int));
    if(!(*sockfd)){
        *sockfd = NULL;
        return 0;
    }
    struct sockaddr_in servaddr;
    **sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (**sockfd <0) {
        free(*sockfd);
        *sockfd = NULL;
        return 0;
    };
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
    if(connect(**sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        close(**sockfd);
        free(*sockfd);
        *sockfd = NULL;
        return 0;
    };
    return 1;
};