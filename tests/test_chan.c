#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <unistd.h>
#include "../include/chan.h"
// Testes unitários criados com a ajuda da ferramenta da IA Deep Seek
// Test sending data to a channel with available space
void test_send_with_available_space() {
    void* queue[10];
    chan* ch = start_chan(10, queue);
    
    int data = 42;
    puts("Hello!!!");
    send(ch, &data);

    assert(ch->queue->front == 0);
    assert(ch->queue->queue[0] == &data);

    remove_chan(ch);
    printf("test_send_with_available_space passed\n");
}
// Thread function to fill the channel
void* fill_channel(void* arg) {
    chan* ch = (chan*)arg;
    int data = 42;
    for (int i = 0; i < ch->queue->capacity; i++) {
        send(ch, &data);
    }
    return NULL;
}

// Thread function to attempt sending to a full channel
void* send_to_full_channel(void* arg) {
    chan* ch = (chan*)arg;
    int data = 43;
    send(ch, &data); // This should block until space is available
    return NULL;
}
// Test sending data to a full channel
void test_send_to_full_channel() {
    void* queue[1]; // Channel with size 1
    chan* ch = start_chan(1, queue);

    pthread_t filler_thread, sender_thread;

    // Create a thread to fill the channel
    pthread_create(&filler_thread, NULL, fill_channel, ch);

    // Wait for the filler thread to finish filling the channel
    pthread_join(filler_thread, NULL);

    // Create a thread to attempt sending to the full channel
    pthread_create(&sender_thread, NULL, send_to_full_channel, ch);

    // Sleep briefly to ensure the sender thread attempts to send
    usleep(100000); // 100ms

    // Verify that the channel is full and the sender thread is blocked
    assert(ch->queue->front == 0);
    assert(ch->queue->size  == 1);
    assert(ch->queue->queue[0] != NULL);

    // Simulate making space in the channel by receiving an item
    void* received_data = receive(ch);
    assert(received_data != NULL);

    // Wait for the sender thread to complete (it should unblock and send the data)
    pthread_join(sender_thread, NULL);

    // Verify that the new data was sent successfully
    assert(ch->queue->front == 0);
    assert(ch->queue->size==1);
    assert(ch->queue->queue[0] != NULL);

    remove_chan(ch);
    printf("test_send_to_full_channel passed\n");
}
int main() {
    //test_send_with_available_space();
    test_send_to_full_channel();
}