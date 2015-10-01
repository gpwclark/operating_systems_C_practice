/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

// TODO REMOVE ID VARIABLE
// TODO turn main into init function
// TODO MUST FREE RET VAL OF INIT!!!!
// TODO finalize the buffer.h
// TODO RENAME sem_conditionals STRUCT, it should be the buffer ADT I think.
// TODO Think critically about asserts... later
// TODO need good semantics so that one thread knows
//      it is the producer or the consumer and will call deposit or remove
//TODO get rid of all comments

#include <time.h> //TODO take this out when done with testing
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//includes for the ST library in CORRECT ORDER, DO NOT REARRANGE
#include "st.h"
#include "buffer.h"
//#include "semaphore.h" included in buffer.h

/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

void deposit(synced_buffer *s_buf, int value){
  down(s_buf->sems->emptyBuffers);
  (s_buf->buffer[s_buf->nextIn]) = value;
  (s_buf->nextIn) = (s_buf->nextIn + 1) % BUFFER_SIZE;
  up(s_buf->sems->fullBuffers);
}

int remoove(synced_buffer *s_buf){
  down(s_buf->sems->fullBuffers);
  int data = (s_buf->buffer[s_buf->nextOut]);
  (s_buf->nextOut) = (s_buf->nextOut + 1) % BUFFER_SIZE;
  up(s_buf->sems->emptyBuffers);
  return data;
}

synced_buffer *buffer_init(int new_id){

  semaphore *emptyBuffers = (semaphore*) malloc(sizeof(semaphore));
  semaphore *fullBuffers = (semaphore*) malloc(sizeof(semaphore));
  createSem(emptyBuffers,BUFFER_SIZE);
  createSem(fullBuffers,0);
  
  //TODO does this need to be malloced since it is going into synced buffer?
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
  sync_buf->id = new_id;
  
  //*shared_buffers = local_buffers;
  return sync_buf;
}
