#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "queue.h"
#ifndef CHAN_H
#define CHAN_H
typedef struct{
    Queue* queue;
    pthread_mutex_t* mutex;
    sem_t* empty_slots;
    sem_t* full_slots;
}chan;

typedef enum {
    ERROR_RECOVERABLE,
    ERROR_FATAL
} ErrorCategory;

typedef struct{
    int error_code;
    ErrorCategory category;
}SendErrorInfo;

typedef struct {
    int error_code;
    ErrorCategory category;
}PthreadMutexLockErrorInfo;
chan*  start_chan(unsigned int size, void** queue);
void** remove_chan(chan* ch);
void*  receive_data(chan* ch);
void   send_data(chan* ch, void* data);
#endif