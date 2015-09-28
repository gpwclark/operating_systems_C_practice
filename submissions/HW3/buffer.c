// TODO BE AWARE OF INCLUDING THINGS TOO MANY TIMES
// TODO put in asserts!!
// TODO do these need to be renamed deposit and remove? it appears that main 
// is definitely not necessary here, plus this file will need to be shared
// between threads so we need good semantics so that one thread knows
// it is the producer or the consumer and will call deposit or remove
// etc.etc.etc.etc

#include <time.h> //TODO take this out when done with testing
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//includes for the ST library in CORRECT ORDER, DO NOT REARRANGE
#include "st.h"
#include "semaphore.h"

//TODO GET THIS OUT OF HERE
#define NUMBER_ITERATIONS 15

// Sleep time for each thread in microseconds.
// TODO GET THIS OUT OF HERE TOOOOO
#define SLEEP_TIME_P 2
#define SLEEP_TIME_C 555

/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

typedef struct {
    semaphore *emptyBuffers;
    semaphore *fullBuffers;
    int *buffer;
}Thread_init;


void producer(Thread_init *init, st_utime_t sleepTime, int nextIn){
    int i;
    int data;
    int value;
    for (i=0; i < NUMBER_ITERATIONS; i++) {

        down(init->emptyBuffers);
        // assert?
        value = rand();
        value = value % 100;
        (init->buffer[nextIn]) = value;
        data = (init->buffer[nextIn]);
        nextIn = (nextIn + 1) % BUFFER_SIZE;
        printf("%s: %d, %s: %d , %s: %d, %s: %d\n","I have produced",data,"my val",value,"i", i,"nextIn",nextIn);
        // assert?
        up(init->fullBuffers);
        // assert?

        st_usleep(sleepTime);
    }

    st_thread_exit(NULL);
}

void consumer(Thread_init *init, st_utime_t sleepTime, int nextOut){
    int i;
    int data;
    for (i=0; i < NUMBER_ITERATIONS; i++) {
      
        down(init->fullBuffers);
        // assert?
        data = (init->buffer[nextOut]);
        nextOut = (nextOut + 1) % BUFFER_SIZE;
        printf("%s: %d, %s: %d, %s: %d\n","I have consumed", data,"i", i,"nextOut",nextOut);
        // assert?
        up(init->emptyBuffers);
        // assert?
        st_usleep(sleepTime);
    }

    st_thread_exit(NULL);
}

void *producer_init(void *init_thread_state) {
    Thread_init *init = init_thread_state;

    //nextIn eval to 0
    int nextIn = 0;
    producer(init, SLEEP_TIME_P, nextIn);      
}

void *consumer_init(void *init_thread_state) {
 
    Thread_init *init = init_thread_state;
    //nextOut eval to 0
    int nextOut = 0;

    consumer(init, SLEEP_TIME_C, nextOut);
}

int main (int argc, char const *argv[]) {

    st_init();
    //TODO when done debugging take this out
    srand(time(NULL));

    semaphore emptyBuffers;
    semaphore fullBuffers;
    createSem(&emptyBuffers,BUFFER_SIZE);
    createSem(&fullBuffers,0);

    //me create buffer, and check for errrors.
    int *buffer = (int*)malloc(sizeof(int) * BUFFER_SIZE); 
    if (buffer == NULL){
      //TODO error message?
      return -1;
    }

    Thread_init shared_buffers = {
        &emptyBuffers,
        &fullBuffers,
        buffer, 
    };

    // Producer Thread 
    if (st_thread_create(producer_init, &shared_buffers, 0, 0) == NULL) {
        perror("Producer thread not spawned: st_thread_create() has failed");
        abort();
    }

    // Consumer Thread
    if (st_thread_create(consumer_init, &shared_buffers, 0, 0) == NULL) {
        perror("Consumer thread not spawned: st_thread_create() has failed");
        abort();
    }

    st_thread_exit(NULL);
    free(buffer);

    return 0;
}
