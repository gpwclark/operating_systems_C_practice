/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

// TODO BE AWARE OF INCLUDING THINGS TOO MANY TIMES
// TODO REMOVE ID VARIABLE
// TODO COMMENT YOUR CODE BUDDY!!!!!!!!!!
/* This class uses a struct, sem_conditionals, this struct is passed to it's two 
 * functions, the sem_conditionals should really be local, because it shouldn't be
 * passed in. On the other hand. When this buffer class is created, deposit
 * will take an int (a char from stdin but it's an int of course) and use
 * that instead of rand, and remoove will return an int. That's the only thing
 * that this class is used for.
 *
 * (buffer->deposit(char));
 * int char = (buffer->remoove());
 */
// TODO turn main into init function
// TODO MUST FREE RET VAL OF INIT!!!!
// TODO remove output stream
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

//TODO GET THIS OUT OF HERE
#define DEF_OUT_STREAM stderr

// Sleep time for each thread in microseconds.
// TODO GET THIS OUT OF HERE TOOOOO
#define SLEEP_TIME_C 222
#define SLEEP_TIME_P 666

/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

int nextIn = 0;
int nextOut = 0;

void deposit(synced_buffer *s_buf, int value){
  int data;
  //int value;
  //st_utime_t sleepTime =  SLEEP_TIME_P;

  fprintf(s_buf->sems->out_stream, "\n%s: %d, %s: %d\n","id",s_buf->id,"1 prod INIT - emptyBuffers",s_buf->sems->emptyBuffers->value); 
  down(s_buf->sems->emptyBuffers);
  fprintf(s_buf->sems->out_stream, "PRODUCER\n");
  fprintf(s_buf->sems->out_stream, "%s: %d,%s: %d\n","2 p emptyBuffers",s_buf->sems->emptyBuffers->value,"id",s_buf->id);
  //assert(s_buf->sems->emptyBuffers->value > 0);

  //value = rand();
  //value = value % 100;
  (s_buf->buffer[s_buf->nextIn]) = value;
  data = (s_buf->buffer[s_buf->nextIn]);
  (s_buf->nextIn) = (s_buf->nextIn + 1) % BUFFER_SIZE;
  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d , %s: %d, %s: %d\n","I have produced",data,"my val",value,"id", s_buf->id,"nextIn",s_buf->nextIn);

  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d\n","3 p fullBuffers",s_buf->sems->fullBuffers->value,"id",s_buf->id);
  up(s_buf->sems->fullBuffers);
  //assert(s_buf->sems->fullBuffers->value > 0);
  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d\n","4 p fullBuffers",s_buf->sems->fullBuffers->value,"id",s_buf->id);
  // assert?
  fflush(s_buf->sems->out_stream);

  //st_usleep(sleepTime);
}

int remoove(synced_buffer *s_buf){
  int data;
  //st_utime_t sleepTime =  SLEEP_TIME_C;

  fprintf(s_buf->sems->out_stream, "\n%s: %d, %s: %d\n","id",s_buf->id,"1 cons INIT - fullBuffers",s_buf->sems->fullBuffers->value);
  down(s_buf->sems->fullBuffers);
  fprintf(s_buf->sems->out_stream, "CONSUMER\n");
  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d\n","2 c fullBuffers",s_buf->sems->fullBuffers->value,"id",s_buf->id);
  //assert(s_buf->sems->fullBuffers->value > 0);

  data = (s_buf->buffer[s_buf->nextOut]);
  (s_buf->nextOut) = (s_buf->nextOut + 1) % BUFFER_SIZE;
  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d, %s: %d\n","I have consumed", data,"id", s_buf->id,"nextOut",s_buf->nextOut);

  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d\n","3 c emptyBuffers",s_buf->sems->emptyBuffers->value,"id",s_buf->id);
  up(s_buf->sems->emptyBuffers);
  fprintf(s_buf->sems->out_stream, "%s: %d, %s: %d\n","4 c emptyBuffers",s_buf->sems->emptyBuffers->value,"id",s_buf->id);
  //assert(s_buf->sems->emptyBuffers->value > 0);
  // assert?

  fflush(s_buf->sems->out_stream);
  //st_usleep(sleepTime);

  return data;
}

synced_buffer *buffer_init(int new_id){

  semaphore *emptyBuffers = (semaphore*) malloc(sizeof(semaphore));
  semaphore *fullBuffers = (semaphore*) malloc(sizeof(semaphore));
  createSem(emptyBuffers,BUFFER_SIZE);
  createSem(fullBuffers,0);

  FILE *out_stream = DEF_OUT_STREAM;
  
  //TODO does this need to be malloced since it is going into synced buffer?
  sem_conditionals *sems = (sem_conditionals*) malloc(sizeof(sem_conditionals));
  if(sems == NULL){
    //return null if malloc fails
    return NULL;
  }

  sems->out_stream = out_stream;  
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
