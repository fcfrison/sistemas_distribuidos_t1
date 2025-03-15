#include <stdlib.h>
#include "../include/queue.h"
//https://www.geeksforgeeks.org/introduction-to-circular-queue/


unsigned char
enqueue(Queue* q, void* el){
    if(q->size==q->capacity){
        return 0;
    };
    unsigned int rear = (q->front+q->size)%q->capacity;
    q->queue[rear] = el;
    q->size++;
    return 1;
};
void*
dequeue(Queue* q){
    if(q->size==0) return 0;
    void* el = q->queue[q->front];
    q->front = (q->front+1)%q->capacity;
    q->size--;
    return el;
};
Queue*
init_queue(unsigned int capacity, void** queue){
    Queue* q = (Queue*)calloc(1,sizeof(Queue));
    if(!q) return NULL;
    q->capacity = capacity;
    q->front = 0;
    q->size =  0;
    q->queue = queue;
    return q;
};
void**
remove_queue(Queue* q){
    if(!q){
        return NULL;
    };
    void** queue = q->queue;
    free(q);
    return queue;
};