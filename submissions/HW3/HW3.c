// TODO BE AWARE OF INCLUDING THINGS TOO MANY TIMES
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
// TODO remove output stream and maybe flush some shit?
// TODO finalize the buffer.h
// TODO Think critically about asserts... later
// TODO need good semantics so that one thread knows
//      it is the producer or the consumer and will call deposit or remove
//TODO separate out buffer and sem_conditionals (which should be a semaphore instead)

#include <time.h> //TODO take this out when done with testing
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//includes for the ST library in CORRECT ORDER, DO NOT REARRANGE
#include "st.h"
//#include "semaphore.h"
#include "sem_conditionals.h"
#include "buffer.h"

//TODO GET THIS OUT OF HERE
#define NUMBER_ITERATIONS 15
#define DEF_OUT_STREAM stdout

// Sleep time for each thread in microseconds.
// TODO GET THIS OUT OF HERE TOOOOO
#define SLEEP_TIME_C 222
#define SLEEP_TIME_P 666

/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

int perform_simple_char_substitutions(char character){
  if(character == '\n'){
    character = ' ';
  }
  return character;
}

void *print_char_buffer(synced_buffer *s_buf){
  int character;
  do{
    character = remoove(s_buf);
    character = perform_simple_char_substitutions(character);
    printf("%d : %c\n",character, character);
  } while (character != EOF);
  st_thread_exit(NULL);
}

void *get_char_stdin(synced_buffer *s_buf){
  int input_char;
  do{
    input_char = fgetc(stdin);
    deposit(s_buf, input_char);
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

// init for print fcn
void *print_char_buffer_init(void *init_buf_state) {
    synced_buffer *s_buf = init_buf_state;
    return print_char_buffer(s_buf);
}
// init for get char stdin
void *get_char_stdin_init(void *init_buf_state) {
    synced_buffer *s_buf = init_buf_state;
    return get_char_stdin(s_buf);
}

int main () {

    st_init();

    synced_buffer *s_buf = buffer_init();
    if (s_buf == NULL){
      printf("Failed to allocate memory for buffer ADT");
    }

    // Producer Thread 
    if (st_thread_create(get_char_stdin_init, s_buf, 0, 0) == NULL) {
        perror("Producer thread not spawned: st_thread_create() has failed");
        abort();
    }

    // Consumer Thread
    if (st_thread_create(print_char_buffer_init, s_buf, 0, 0) == NULL) {
        perror("Consumer thread not spawned: st_thread_create() has failed");
        abort();
    }

    st_thread_exit(NULL);
    //TODO since this never gets called, free the buffer elsewhere?
    free(s_buf);
    printf("I'm done!!");

    return 0;
}
