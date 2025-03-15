#ifndef QUEUE_H
#define QUEUE_H
typedef struct{
    void** queue;
    unsigned int capacity;
    unsigned int size;
    unsigned int front;
}Queue;
unsigned char enqueue(Queue* q, void* el);
void*         dequeue(Queue* q);
Queue*        init_queue(unsigned int max_size, void** queue);
void**        remove_queue(Queue* q);
#endif