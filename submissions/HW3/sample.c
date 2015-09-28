// TODO BE AWARE OF INCLUDING THINGS TOO MANY TIMES

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //TODO take this out when done with texting

// Library and custom includes
#include "st.h"
#include "semaphore.h"

// Output-related defaults (stream, message, shared value length)
#define DEFAULT_OUTPUT_STREAM stdout
#define DEFAULT_AGREE_MSG "                                    [thread %c]: See? Homework is %s!\n"
#define DEFAULT_DISAGREE_MSG "                                 [thread %c]: Thread %c is lame. Homework is %s!\n"
#define DEFAULT_MSG_A "great\0"
#define DEFAULT_MSG_B "boring\0"
#define MAX_LEN_SHARED_MSG 12
// How many times to iterate in each thread.
#define NUMBER_ITERATIONS 15
// Sleep time for each thread in microseconds.
/*
#define SLEEP_TIME_A 333333          
#define SLEEP_TIME_B 666666
*/
#define SLEEP_TIME_A 333333
#define SLEEP_TIME_B 666666 

// Thread identifiers
#define THREAD_A_ID 'A'
#define THREAD_B_ID 'B'

//me define buffer size
#define BUFFER_SIZE 80

// Initialization data structure for our threads
//TODO 
typedef struct {

    FILE *output_stream;        // Output target

    //me set up the buffer semaphores and actual buffer
    semaphore *mutex;           // Pointer to a mutex
    semaphore *emptyBuffers;
    semaphore *fullBuffers;
    int *buffer;

    char *shared_value_ptr;     // Pointer to a shared string
    char *last_set_by;          // Identifier of the thread who last set

} ThreadInit;

// Prototypes for functions constituting execution of each thread.
void *thread_a_func(void *state);
void *thread_b_func(void *state);
void *producer_logic(ThreadInit *init, char thread_id, char *my_msg, st_utime_t sleepTime, int nextIn);
void *consumer_logic(ThreadInit *init, char thread_id, char *my_msg, st_utime_t sleepTime, int nextOut);

/*
 * Create two threads using the ST library.
 * The two threads simply compete by setting the value of shared_value.
 * Execution is controlled by sleeping each thread after it modifies the value.
 * Execution stops after a number of iterations defined above.
 */
int main (int argc, char const *argv[]) {

    // Initialize the libST runtime.
    st_init();
    srand(time(NULL));
    // Create space for a MAX_LEN_SHARED_MSG-character string.
    char shared_value[MAX_LEN_SHARED_MSG];
    strncpy(shared_value, DEFAULT_MSG_A, MAX_LEN_SHARED_MSG);

    // Create char for last updater
    char last_set_by = THREAD_A_ID;

    //me Create and initialize a mutex to 1.
    semaphore mutex;
    semaphore emptyBuffers;
    semaphore fullBuffers;
    createSem(&mutex, 1);
    createSem(&emptyBuffers,BUFFER_SIZE);
    createSem(&fullBuffers,0);

    //me create buffe
    //TODO FREE THE BUFFER< FREE IT >
    int *buffer = (int*)malloc(sizeof(int) * BUFFER_SIZE); 
    if (buffer == NULL){
      //TODO error message?
      return -1;
    }

    // Set the file descriptor for output
    FILE *output_stream = DEFAULT_OUTPUT_STREAM;

    // Create the struct used to initialize our threads.
    ThreadInit init = {
        output_stream,
        &mutex,
        &emptyBuffers,
        &fullBuffers,
        buffer, 
        shared_value,
        &last_set_by
    };

    // Create thread A
    if (st_thread_create(thread_a_func, &init, 0, 0) == NULL) {
        perror("st_thread_create failed for thread a");
        exit(EXIT_FAILURE);
    }

    // Create thread B
    if (st_thread_create(thread_b_func, &init, 0, 0) == NULL) {
        perror("st_thread_create failed for thread b");
        exit(EXIT_FAILURE);
    }

    // Exit from main via ST.
    st_thread_exit(NULL);
    return 0;

}

// The execution path for thread a
void *thread_a_func(void *state) {
    // Appease the compiler by casting the input
    ThreadInit *init = state;

    // The identifier for this thread
    char thread_id = THREAD_A_ID;
    // The value that this thread wants to set
    char *my_msg = DEFAULT_MSG_A;

    //nextIn eval to 0
    int nextIn = 0;
    return producer_logic(init, thread_id, my_msg, SLEEP_TIME_A, nextIn);      
}

// The execution path for threa d
void *thread_b_func(void *state) {
    // Appease the compiler by casting the input
    ThreadInit *init = state;

    // The identifier for this thread
    char thread_id = THREAD_B_ID;
    // The value that this thread wants to set
    char *my_msg = DEFAULT_MSG_B;
    
    //nextOut eval to 0
    int nextOut = 0;

    return consumer_logic(init, thread_id, my_msg, SLEEP_TIME_B, nextOut);
}

// The logic common to each thread: read the shared value and complain and update or celebrate.
void *producer_logic(ThreadInit *init, char thread_id, char *my_msg, st_utime_t sleepTime, int nextIn){
    int i;
    int data;
    int value;
    for (i=0; i < NUMBER_ITERATIONS; i++) {

        // me producer logic?
        int blocked = 0;
        down(init->emptyBuffers);
        blocked = 1;
        value = rand();
        value = value % 100;
        (init->buffer[nextIn]) = value;
        data = (init->buffer[nextIn]);
        nextIn = (nextIn + 1) % BUFFER_SIZE;
        printf("%s: %d, %s: %d , %s: %d, %s: %d\n","I have produced",data,"my val",value,"i", i,"nextIn",nextIn);
        up(init->fullBuffers);

        if (blocked == 0){
          printf("THREAD A blocked\n");
        }else{
          printf("THREAD A ran\n");
        }
        blocked = 0;
        

/*
        // Acquire the mutex by decrementing it.
        down(init->mutex);

        // Read the shared value.
        // If the thread sees anything but it's own value, it will complain.
        // It will also change the shared value to its preferred value.
        if (strncmp(init->shared_value_ptr, my_msg, MAX_LEN_SHARED_MSG) != 0) {
            fprintf(init->output_stream, DEFAULT_DISAGREE_MSG, thread_id, *(init->last_set_by), my_msg);
            strncpy(init->shared_value_ptr, my_msg, MAX_LEN_SHARED_MSG);
            *(init->last_set_by) = thread_id;
        } else { // Otherwise, the thread will indicate that it was right.
            fprintf(init->output_stream, DEFAULT_AGREE_MSG, thread_id, init->shared_value_ptr);
        }
        // Either way, flush the output stream.
        fflush(init->output_stream);

        // Now release the mutex and sleep for some time.
        up(init->mutex);
  */
        st_usleep(sleepTime);

    }

    // Finally, exit once NUMBER_ITERATIONS have been performed
    st_thread_exit(NULL);
}

void *consumer_logic(ThreadInit *init, char thread_id, char *my_msg, st_utime_t sleepTime, int nextOut){
    int i;
    int data;
    for (i=0; i < NUMBER_ITERATIONS; i++) {
      
        //me consumer logic?
        int blocked = 0;
        down(init->fullBuffers);
        blocked = 1;
        data = (init->buffer[nextOut]);
        nextOut = (nextOut + 1) % BUFFER_SIZE;
        printf("%s: %d, %s: %d, %s: %d\n","I have consumed", data,"i", i,"nextOut",nextOut);
        up(init->emptyBuffers);

        if (blocked == 0){
          printf("THREAD B blocked\n");
        }else{
          printf("THREAD B ran\n");
        }
        
        blocked = 0;
        // Acquire the mutex by decrementing it.
  /*
        down(init->mutex);

        // Read the shared value.
        // If the thread sees anything but it's own value, it will complain.
        // It will also change the shared value to its preferred value.
        if (strncmp(init->shared_value_ptr, my_msg, MAX_LEN_SHARED_MSG) != 0) {
            fprintf(init->output_stream, DEFAULT_DISAGREE_MSG, thread_id, *(init->last_set_by), my_msg);
            strncpy(init->shared_value_ptr, my_msg, MAX_LEN_SHARED_MSG);
            *(init->last_set_by) = thread_id;
        } else { // Otherwise, the thread will indicate that it was right.
            fprintf(init->output_stream, DEFAULT_AGREE_MSG, thread_id, init->shared_value_ptr);
        }
        // Either way, flush the output stream.
        fflush(init->output_stream);

        // Now release the mutex and sleep for some time.
        up(init->mutex);
    */
        st_usleep(sleepTime);

    }

    // Finally, exit once NUMBER_ITERATIONS have been performed
    st_thread_exit(NULL);
}
