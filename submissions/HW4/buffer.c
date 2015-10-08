/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "st.h"
#include "buffer.h"

//TODO what happens when I return NULL again?
//TODO remove semaphore crap

/* MAIN EXPLANATION
 * The buffer.c class is based on specs in the buffer.h file.
 * There is a buffer ADT that has a copy of the semaphores used by deposit
 * and remoove as well as a copy of the buffer that two threads share to
 * operate on deposit and remoove. The init function handles the set up
 * of the buffer ADT. The caller is responsible for freeing the memory
 * malloced by the init function.
 */


/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

/*
 * The deposit function in the producer - consumer paradigm uses these semaphore
 * values to keep track of how much data it has in it's "circular" buffer. Deposit
 * asserts that there are at least some buffers free before it inputs data, if
 * the buffer is full, it spins until it isn't. When it's done, it increments 
 * the fullBuffers semaphore indicating that there is now data in the buffer.
 */
void deposit(synced_buffer *s_buf, int value){
  st_sleep(1);
  fprintf(stderr,"about to write %d\n", value);
  write(*(s_buf->write_pipe), &value, sizeof(int));
}

/*
 * The remoove function in the producer - consumer paradigm uses these semaphore
 * values to keep track of how much data it has in it's "circular" buffer. Remoove 
 * asserts that there are at least some buffers full before it removes data, if
 * the buffer is empty, it spins until it isn't. When it's done, it increments 
 * the emptyBuffers semaphore indicating that it remooved data from the buffer.
 */
int remoove(synced_buffer *s_buf){
  st_sleep(1);
  int character;
  read(*(s_buf->read_pipe), &character, sizeof(int));
  fprintf(stderr,"just read %d\n", character);
  return character;
}

synced_buffer *buffer_init(){

  synced_buffer *sync_buf = (synced_buffer*) malloc(sizeof(synced_buffer));
  if(sync_buf == NULL){
    //return null if malloc fails
    return NULL;
  }
  
  int *myPipe[2];
  
  if (pipe(*myPipe) == -1){
    fprintf(stderr, "Failed to create a pipe. terminating.");
    return NULL;
  }

  sync_buf->read_pipe = myPipe[0];
  sync_buf->write_pipe = myPipe[1];
  
  return sync_buf;
}
