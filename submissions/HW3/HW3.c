/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

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
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

//includes for the ST library in CORRECT ORDER, DO NOT REARRANGE
#include "st.h"
//#include "semaphore.h"
#include "sem_conditionals.h"
#include "buffer.h"

/* Since this is a pipeline (takes ints, returns ints) I had to make the 
 * is_valid_input function into something that would return an int, rather
 * than a boolean from hw1. I'm not happy with this solution and the define
 * using -2 as the bad value, nor am I happy I have to check it every time,
 * but it happens. 
 */
#define INVALID_INPUT -2
#define LINE_PRINT_LENGTH 80

/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200
//TODO GET THIS OUT OF HERE
#define NUMBER_ITERATIONS 15
#define DEF_OUT_STREAM stdout

// Sleep time for each thread in microseconds.
// TODO GET THIS OUT OF HERE TOOOOO
#define SLEEP_TIME_C 222
#define SLEEP_TIME_P 666

typedef struct{
  synced_buffer *s_buf_cons;
  synced_buffer *s_buf_prod;
  int (*function_ptr)(int);
} hybrid_buffer;

typedef struct{
  synced_buffer *s_buf;
  int (*function_ptr)(int); 
} producer_buffer;

typedef struct{
  synced_buffer *s_buf;
  int (*function_ptr)(int);
} consumer_buffer;

// global variables.
int curr_char_count = 0;
int *print_buffer;

int perform_simple_char_subs(int character){
  if(character == '\n'){
    character = ' ';
  }
  return character;
}

/* To abstract away what valid input is, should it change, this function checks
 * to make sure the character from the input stream is something to process:
 * printable characters or valid whitespace characters
 */
int is_valid_input(int character){
  if(isprint(character) || isspace(character)){
    return character;
  }else{
    return INVALID_INPUT;
  }
}

// Gets the user input from stdin
int get_user_input(int var_im_sad_about){
  return fgetc(stdin);
}

void *hybrid(void *init_hybrid_buf){
  hybrid_buffer *hybrid_buf = init_hybrid_buf;
  int input_char;
  do{
    input_char = remoove(hybrid_buf->s_buf_cons);
    input_char = (*(hybrid_buf->function_ptr))(input_char);
    if (input_char != INVALID_INPUT){
      deposit(hybrid_buf->s_buf_prod, input_char);
    }
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

// this is the consumer
void *consumer(void *init_cons_buf){
  consumer_buffer *cons_buf = init_cons_buf;

  int input_char;
  do{
    input_char = remoove(cons_buf->s_buf);
    input_char = (*(cons_buf->function_ptr))(input_char);
    printf("%d : %c\n",input_char, input_char);
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

// this is the producer
void *producer(void *init_prod_buf){
  producer_buffer *prod_buf = init_prod_buf;

  int input_char;
  do{
    input_char = (*(prod_buf->function_ptr))(input_char);
    deposit(prod_buf->s_buf, input_char);
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

/* don't think we need these...
// this is the consumer init function
void *print_char_buffer_init(void *init_buf_state) {
    synced_buffer *s_buf = init_buf_state;
    return print_char_buffer(s_buf);
}
// this is the producer init function
void *get_char_stdin_init(void *init_buf_state) {
    synced_buffer *s_buf = init_buf_state;
    return get_char_stdin(s_buf);
}
*/
int main () {
/*
    print_buffer = (int *) malloc(sizeof(int)*PRINT_LINE_LENGTH);
    if(print_buffer == NULL){
       return -1;
    }
    */

    st_init();
   /* 
    int (*fcn_ptr[5])(int);
    fcn_ptr[0] = get_user_input;
    fcn_ptr[1] = is_invalid_input;
    fcn_ptr[2] = perform_simple_char_subs;
    fcn_ptr[3] = perform_complex_char_subs;
    fcn_ptr[4] = ;
    */
    
    synced_buffer *s_buf = buffer_init();
    if (s_buf == NULL){
      printf("Failed to allocate memory for buffer ADT");
    }

    producer_buffer *prod_buf = (producer_buffer*) malloc(sizeof(producer_buffer));
    consumer_buffer *cons_buf = (consumer_buffer*) malloc(sizeof(consumer_buffer));

    prod_buf->s_buf = s_buf;
    prod_buf->function_ptr = &get_user_input;
    
    cons_buf->s_buf = s_buf;
    cons_buf->function_ptr = &perform_simple_char_subs;

    // Producer Thread 
    if (st_thread_create(producer, prod_buf, 0, 0) == NULL) {
        perror("Producer thread not spawned: st_thread_create() has failed");
        abort();
    }

    // Consumer Thread
    if (st_thread_create(consumer, cons_buf, 0, 0) == NULL) {
        perror("Consumer thread not spawned: st_thread_create() has failed");
        abort();
    }

    st_thread_exit(NULL);
    //TODO since this never gets called, free the buffer elsewhere?
    //     OR IS THE KEY IN THIS JOINING BUSINESS?
    free(prod_buf);
    free(cons_buf);
    printf("I'm done!!");

    return 0;
}
