/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "st.h"
#include "buffer.h"

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
  down(s_buf->sems->emptyBuffers);
  // assert that emptyBuffers != 0
  (s_buf->buffer[s_buf->nextIn]) = value;
  (s_buf->nextIn) = (s_buf->nextIn + 1) % BUFFER_SIZE;
  // assert buffer now has data
  up(s_buf->sems->fullBuffers);
}

/*
 * The remoove function in the producer - consumer paradigm uses these semaphore
 * values to keep track of how much data it has in it's "circular" buffer. Remoove 
 * asserts that there are at least some buffers full before it removes data, if
 * the buffer is empty, it spins until it isn't. When it's done, it increments 
 * the emptyBuffers semaphore indicating that it remooved data from the buffer.
 */
int remoove(synced_buffer *s_buf){
  down(s_buf->sems->fullBuffers);
  // assert that fullBuffers != 0
  int data = (s_buf->buffer[s_buf->nextOut]);
  (s_buf->nextOut) = (s_buf->nextOut + 1) % BUFFER_SIZE;
  // assert buffer has had data removed
  up(s_buf->sems->emptyBuffers);
  return data;
}

synced_buffer *buffer_init(){
  
  semaphore *emptyBuffers = (semaphore*) malloc(sizeof(semaphore));
  if(emptyBuffers == NULL){
    //return null if malloc fails
    return NULL;
  }

  semaphore *fullBuffers = (semaphore*) malloc(sizeof(semaphore));
  if(fullBuffers == NULL){
    //return null if malloc fails
    return NULL;
  }

  // Initialize semaphores for use in remoove and deposit.
  createSem(emptyBuffers,BUFFER_SIZE);
  createSem(fullBuffers,0);
  
  // Create the semaphore object for this particular buffer
  sem_conditionals *sems = (sem_conditionals*) malloc(sizeof(sem_conditionals));
  if(sems == NULL){
    //return null if malloc fails
    return NULL;
  }

  sems->emptyBuffers = emptyBuffers;
  sems->fullBuffers = fullBuffers;

  //me create buffer, and check for errrors.
  int *buffer = (int*)malloc(sizeof(int) * BUFFER_SIZE); 
  if (buffer == NULL){
    //TODO what is EXIT_FAILURE?
    // exit(EXIT_FAILURE);
    return NULL;
  }

  synced_buffer *sync_buf = (synced_buffer*) malloc(sizeof(synced_buffer));
  if(sync_buf == NULL){
    //return null if malloc fails
    return NULL;
  }
  
  sync_buf->buffer = buffer;
  sync_buf->sems = sems;
  sync_buf->nextIn = 0;
  sync_buf->nextOut = 0;
  
  return sync_buf;
}
