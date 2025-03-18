#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include "../include/chan.h"
#include "../include/errors.h"
//--------------------------------------------------------//
//---------------- CHANNEL IMPLEMENTATION ----------------//
//--------------------------------------------------------//

/**
 * Esta implementacao não é confiável para o caso em que o 
 * buffer do canal for igual a zero.
 */

PthreadMutexLockErrorInfo* categorize_mtx_lck_error(int err_no);
int  msleep(long msec,int max_retries);
void
mutex_lock(pthread_mutex_t* mtx,
           unsigned int     n_retries,
           unsigned int     retry_tm){
    int rtn;
    rtn = pthread_mutex_lock(mtx);
    if(rtn==0){
        return;
    }
    perror("Error: ");
    PthreadMutexLockErrorInfo* err_info = NULL;
    for(size_t i = 0; i < n_retries; i++){
        if(err_info){
            free(err_info);
        }
        err_info = categorize_mtx_lck_error(rtn);
        if(!err_info){
            exit(EXIT_FAILURE);
        }
        if((err_info->category==ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
            free(err_info);
            exit(EXIT_FAILURE);
        }
        rtn = pthread_mutex_lock(mtx);
        if(rtn==0){
            free(err_info);
            return;
            }
        perror("Error: ");
    }
    if(err_info){
        free(err_info);
    }
    exit(EXIT_FAILURE);
};
void 
mutex_unlock(pthread_mutex_t* mtx){
    int rtn;
    rtn = pthread_mutex_unlock(mtx);
    if(rtn==0){
        return;
    }
    perror("Error: ");
    exit(EXIT_FAILURE);
};

PthreadMutexLockErrorInfo*
categorize_mtx_lck_error(int err_no){
    PthreadMutexLockErrorInfo* err = (PthreadMutexLockErrorInfo*) calloc(1,sizeof(PthreadMutexLockErrorInfo));
    switch (err_no){
    case EOWNERDEAD:
        err->category = ERROR_RECOVERABLE;
        break;
    
    default:
        err->category = ERROR_FATAL;
        break;
    }
    return err;
}
int 
msleep(long msec,int max_retries){
    struct timespec ts;
    int res;
    ts.tv_sec  =  msec / 1000;            //sleep time in seconds
    ts.tv_nsec = (msec % 1000) * 1000000; // sleep time in nanoseconds
    errno      = 0;
    unsigned char retries;
    do{
        res = nanosleep(&ts, NULL);
        if(res!=-1){
            return res;
        }
        char* msg = "Error: ";
        perror(msg);
        switch (errno){
            case EINTR: 
                retries++;
                errno = 0; 
                break; 
            default: 
                return res;
        }
    }while(retries<max_retries);
    return res;
}
// Send data to the channel
void 
send_data(chan* ch, void* data){
    sem_wait(ch->empty_slots);
    mutex_lock(ch->mutex,3,1000);
    enqueue(ch->queue,data);
    mutex_unlock(ch->mutex);
    sem_post(ch->full_slots);
    return;
};
// Receive data from the channel
void*
receive_data(chan* ch){
    sem_wait(ch->full_slots);
    mutex_lock(ch->mutex,3,1000);
    void* el = dequeue(ch->queue);
    mutex_unlock(ch->mutex);
    sem_post(ch->empty_slots);
    return el;
}
chan*
start_chan(unsigned int size, void** queue){
    chan* ch        = (chan*)calloc(1,sizeof(chan));
    ch->queue       = init_queue(size, queue);
    // initialize mutex
    ch->mutex       = (pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(ch->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    // initialize semaphores
    ch->empty_slots = (sem_t*)calloc(1,sizeof(sem_t));
    ch->full_slots  = (sem_t*)calloc(1,sizeof(sem_t));
    sem_init(ch->empty_slots, 0, size);
    sem_init(ch->full_slots, 0, 0);
    return ch;
}
void**
remove_chan(chan* ch){
    if(!ch) return NULL;
    if(ch->empty_slots){
        free(ch->empty_slots);
    };
    if(ch->full_slots){
        free(ch->full_slots);
    };
    void** queue = remove_queue(ch->queue);
    free(ch);
    return queue;
}
