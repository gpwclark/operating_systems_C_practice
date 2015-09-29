// TODO BE AWARE OF INCLUDING THINGS TOO MANY TIMES
// TODO COMMENT YOUR CODE BUDDY!!!!!!!!!!
/* This class uses a struct, Thread_inst, this struct is passed to it's two 
 * functions, the Thread_inst should really be local, because it shouldn't be
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
// TODO RENAME THREAD_INST STRUCT, it should be the buffer ADT I think.
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

void deposit(Thread_inst *thread, int value){
  int i;
  int data;
  //int value;
  int nextIn = 0;
  //st_utime_t sleepTime =  SLEEP_TIME_P;

  fprintf(thread->out_stream, "\n%s: %d\n","1 prod INIT - emptyBuffers",thread->emptyBuffers->value); 
  down(thread->emptyBuffers);
  fprintf(thread->out_stream, "PRODUCER\n");
  fprintf(thread->out_stream, "%s: %d\n","2 p emptyBuffers",thread->emptyBuffers->value);
  //assert(thread->emptyBuffers->value > 0);

  //value = rand();
  //value = value % 100;
  (thread->buffer[nextIn]) = value;
  data = (thread->buffer[nextIn]);
  nextIn = (nextIn + 1) % BUFFER_SIZE;
  fprintf(thread->out_stream, "%s: %d, %s: %d , %s: %d, %s: %d\n","I have produced",data,"my val",value,"i", i,"nextIn",nextIn);

  fprintf(thread->out_stream, "%s: %d\n","3 p fullBuffers",thread->fullBuffers->value);
  up(thread->fullBuffers);
  //assert(thread->fullBuffers->value > 0);
  fprintf(thread->out_stream, "%s: %d\n","4 p fullBuffers",thread->fullBuffers->value);
  // assert?
  fflush(thread->out_stream);

  // st_usleep(sleepTime);
}

int remoove(Thread_inst *thread){
  int i;
  int data;
  int nextOut = 0;
  //st_utime_t sleepTime =  SLEEP_TIME_C;

  fprintf(thread->out_stream, "\n%s: %d\n","1 cons INIT - fullBuffers",thread->fullBuffers->value);
  down(thread->fullBuffers);
  fprintf(thread->out_stream, "CONSUMER\n");
  fprintf(thread->out_stream, "%s: %d\n","2 c fullBuffers",thread->fullBuffers->value);
  //assert(thread->fullBuffers->value > 0);

  data = (thread->buffer[nextOut]);
  nextOut = (nextOut + 1) % BUFFER_SIZE;
  fprintf(thread->out_stream, "%s: %d, %s: %d, %s: %d\n","I have consumed", data,"i", i,"nextOut",nextOut);

  fprintf(thread->out_stream, "%s: %d\n","3 c emptyBuffers",thread->emptyBuffers->value);
  up(thread->emptyBuffers);
  fprintf(thread->out_stream, "%s: %d\n","4 c emptyBuffers",thread->emptyBuffers->value);
  //assert(thread->emptyBuffers->value > 0);
  // assert?

  fflush(thread->out_stream);
  //st_usleep(sleepTime);

  return data;
}

Thread_inst *buffer_init(){
  semaphore *emptyBuffers = (semaphore*) malloc(sizeof(semaphore));
  semaphore *fullBuffers = (semaphore*) malloc(sizeof(semaphore));
  /*
  semaphore emptyBuffers;
  semaphore fullBuffers;
  */

  createSem(emptyBuffers,BUFFER_SIZE);
  createSem(fullBuffers,0);

  FILE *out_stream = DEF_OUT_STREAM;

  //me create buffer, and check for errrors.
  int *buffer = (int*)malloc(sizeof(int) * BUFFER_SIZE); 
  if (buffer == NULL){
    //TODO what is EXIT_FAILURE?
    exit(EXIT_FAILURE);
  }

  Thread_inst *shared_buffers = (Thread_inst*) malloc(sizeof(Thread_inst));
  if(buffer == NULL){
    //return null if malloc fails
    return NULL;
  }

  shared_buffers->out_stream = out_stream;  
  shared_buffers->emptyBuffers = emptyBuffers;
  shared_buffers->fullBuffers = fullBuffers;
  shared_buffers->buffer = buffer;
  
  //*shared_buffers = local_buffers;
  return shared_buffers;
}
