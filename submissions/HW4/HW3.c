/* This is a pledge. A pledge. Of honor. It's pretty good. Honor pledges are.
 * Generally supposed. To have short. Choppy. Sentences. This assignment was
 * hard. Especially when you write bugs in your buffer.c. But now I'm done,
 * and now that I am, I would like to say that I have neither given or 
 * received help on this program.
 * George Clark
 */ 


/* Main explanation:
 * This program creates a bunch of thread objects that share buffers 
 * between them. Each thread has an assigned function, and those function
 * are evaluated in each thread and their IO is shared in a pipeline
 * via the shared buffers. There are 5 threads and 4 shared buffers
 * between them. The first thread gets data from the user, the second
 * thread verifies the input is valid, the third thread performs a 
 * char substitution, the fourth thread performs a char substitution,
 * and the last thread prints the contents of a global buffer, made
 * up of data it read from its shared buffer iff its special buffer
 * is 80 characters long.
 */

// TODO check arbitrarily long input use yes cmd
// TODO make another pass at using asterisk_found variable
// TODO use getline()?
// TODO free memory

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
#define FCN_ARRAY_LEN 5
#define GET_INPUT_IX 0
#define VALID_IN_IX 1
#define SIMPLE_SUBS_IX 2
#define CMPLX_SUBS_IX 3
#define PRINT_OUT_IX 4

/* Each of these structs holds a function in the pipeline, and the buffers
 * it needs. Producers and consumers only need one buffer, while hybrid
 * buffers need two, so they can consume from one and produce to another.
 */

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

// global variables. really just used by print_to_stdout
int curr_char_count = 0;
int *print_buffer;

/*
 * This function prints the buffer to stdout and appends
 * the newline character to the end. 
 */
int print_to_stdout(int character){
  if (character != EOF){
    print_buffer[curr_char_count] = character;
    ++curr_char_count; 

    int i;
    if (curr_char_count == (LINE_PRINT_LENGTH) ){
      for(i = 0; i < LINE_PRINT_LENGTH; ++i){
        printf("%c", print_buffer[i]);
      }
      printf("\n");
      curr_char_count = 0;
    }
  }
  return character;
}        

int perform_complex_char_subs(int character){
  // I used to have a global boolean variable that
  // used logic in this function to make the ^ sub
  // but this thread couldn't see the variable for
  // some reason so there was just nothing to be 
  // done. This is essentially a get and return now.
  return character;
}

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
    if(character == EOF){
      return EOF;
    }
    return INVALID_INPUT;
  }
}

// Gets the user input from stdin
int get_user_input(int character){
  character = fgetc(stdin);
  return character;
}

/* hybrid handles the logic for three functions, as a result
 * it is a bit messy. Essentially, it has to read in a char,
 * verify it is valid input, then check to see if there is an
 * asterisk, handle the ** -> ^ squelch, as well as verify
 * that EOF doesn't muck things up.
 */
void *hybrid(void *init_hybrid_buf){
  hybrid_buffer *hybrid_buf = init_hybrid_buf;
  int input_char;
  int input_char1;
  do{
    input_char = remoove(hybrid_buf->s_buf_cons);
    input_char = (*(hybrid_buf->function_ptr))(input_char);


    // When the fcn ptr = is_invalid_input, it will set the INVALID_INPUT
    // flag, and skip any depositing this round.
    if (input_char != INVALID_INPUT){

      // If it is an asterisk we need to check the next item in the buffer.
      if (input_char == '*'){

        // set asterisk_found = true so iff there is a subsequent asterisk
        // perform_complex_char_subs will return a ^ char.
        input_char1 = remoove(hybrid_buf->s_buf_cons);
        if(input_char1 == '*'){
          input_char1 = '^';
        }

        if(input_char1 != EOF){

          // If ^ is returned it was a double ** so we deposit it, otherwise
          // deposit both characters, false alarm, only a single asterisk.
          if (input_char1 == '^'){
            deposit(hybrid_buf->s_buf_prod, input_char1);
          } else {
            deposit(hybrid_buf->s_buf_prod, input_char);
            deposit(hybrid_buf->s_buf_prod, input_char1);
          }
          // if the next input char was EOF, get chars in buffer and exit loop
        } else {
          deposit(hybrid_buf->s_buf_prod, input_char);
          deposit(hybrid_buf->s_buf_prod, input_char1);
          break;
        }

      } else { //not an asterisk so no special ops, just deposit.
        deposit(hybrid_buf->s_buf_prod, input_char);
      }
    }//only execute body if valid input

  } while (input_char != EOF);
  st_thread_exit(NULL);
}

/* this is the consumer, very simple job, consume and then pass data to fcn,
 * in this case it is just feeding things to print_to_stdout
 */

void *consumer(void *init_cons_buf){
  consumer_buffer *cons_buf = init_cons_buf;

  int input_char;
  do{
    input_char = remoove(cons_buf->s_buf);
    input_char = (*(cons_buf->function_ptr))(input_char);

    if(input_char==EOF){
      break;
    }
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

/* this is the producer, also has a very simple job, get data from user
 * and then put it in a buffer.
 */
void *producer(void *init_prod_buf){
  producer_buffer *prod_buf = init_prod_buf;

  int input_char;
  do{
    input_char = (*(prod_buf->function_ptr))(input_char);
    deposit(prod_buf->s_buf, input_char);
  } while (input_char != EOF);
  st_thread_exit(NULL);
}

int main () {

  print_buffer = (int *) malloc(sizeof(int)*LINE_PRINT_LENGTH);
  if (print_buffer == NULL){
    printf("malloc failed");
    return -1;
  }

  st_init();

  int (*fcn_ptr[FCN_ARRAY_LEN])(int);
  fcn_ptr[GET_INPUT_IX] = get_user_input;
  fcn_ptr[VALID_IN_IX] = is_valid_input;
  fcn_ptr[SIMPLE_SUBS_IX] = perform_simple_char_subs;
  fcn_ptr[CMPLX_SUBS_IX] = perform_complex_char_subs;
  fcn_ptr[PRINT_OUT_IX] = print_to_stdout;
 
  /*
   * The logic here got a bit munged up and it could probably be put in a loop
   * but this explicit style lent itself better to testing different pieces.
   * 
   * A synced buffer is the buffer that all of the threads need. Plain and simple.
   * Each function is in one thread, and there are four buffers being shared b/w
   * them all.
   */
  synced_buffer *s_buf_1 = buffer_init();
  synced_buffer *s_buf_2 = buffer_init();
  synced_buffer *s_buf_3 = buffer_init();
  synced_buffer *s_buf_4 = buffer_init();

  if (s_buf_1 == NULL || s_buf_2 == NULL ||  s_buf_3 == NULL || s_buf_4 == NULL){
    printf("Failed to allocate memory for buffer ADT");
    return -1;
  }

  /* Relation 1
   * the producer buffer and the first hybrid buffer are malloced, they are given a shared
   * buffer (prod is obviously the producer so the hybrid is the consumer in this relationship
   * and then they are given their assigned functions for execution.
   */
  producer_buffer *prod_buf = (producer_buffer*)malloc(sizeof(producer_buffer));
  if (prod_buf == NULL){
    printf("malloc failed");
    return -1;
  }
  hybrid_buffer *hybrid_buf_a = (hybrid_buffer*)malloc(sizeof(hybrid_buffer));
  if (hybrid_buf_a == NULL){
    printf("malloc failed");
    return -1;
  }

  prod_buf->s_buf = s_buf_1;
  hybrid_buf_a->s_buf_cons = s_buf_1;

  prod_buf->function_ptr = fcn_ptr[GET_INPUT_IX];
  hybrid_buf_a->function_ptr = fcn_ptr[VALID_IN_IX];

  /* Relation 2
   * Pattern is the same as relation 1 except the hybrid buffer declared in
   * relation 1 is the producer in this scenario. Which makes the next hybrid
   * buffer the consumer here
   */
  hybrid_buffer *hybrid_buf_b = (hybrid_buffer*)malloc(sizeof(hybrid_buffer));
  if (hybrid_buf_b == NULL){
    printf("malloc failed");
    return -1;
  }

  hybrid_buf_a->s_buf_prod = s_buf_2;
  hybrid_buf_b->s_buf_cons = s_buf_2;

  hybrid_buf_b->function_ptr = fcn_ptr[SIMPLE_SUBS_IX];

  //Relation 3 - same pattern as Relation 2.
  hybrid_buffer *hybrid_buf_c = (hybrid_buffer*)malloc(sizeof(hybrid_buffer));
  if (hybrid_buf_c == NULL){
    printf("malloc failed");
    return -1;
  }
  hybrid_buf_b->s_buf_prod = s_buf_3;
  hybrid_buf_c->s_buf_cons = s_buf_3;

  hybrid_buf_c->function_ptr = fcn_ptr[CMPLX_SUBS_IX];

  // Relation 4 - same pattern as relation 2 except consumer is not a hybrid.
  consumer_buffer *cons_buf = (consumer_buffer*)malloc(sizeof(consumer_buffer));
  if (cons_buf == NULL){
    printf("malloc failed");
    return -1;
  }

  hybrid_buf_c->s_buf_prod = s_buf_4;

  cons_buf->s_buf = s_buf_4;
  cons_buf->function_ptr = fcn_ptr[PRINT_OUT_IX];

  // SEND THEM OFFF!!!
  if (st_thread_create(producer, prod_buf, 0, 0) == NULL) {
    perror("Producer thread not spawned: st_thread_create() has failed");
    abort();
  }
  if (st_thread_create(hybrid, hybrid_buf_a, 0, 0) == NULL) {
    perror("Consumer thread not spawned: st_thread_create() has failed");
    abort();
  }

  if (st_thread_create(hybrid, hybrid_buf_b, 0, 0) == NULL) {
    perror("Consumer thread not spawned: st_thread_create() has failed");
    abort();
  }

  if (st_thread_create(hybrid, hybrid_buf_c, 0, 0) == NULL) {
    perror("Consumer thread not spawned: st_thread_create() has failed");
    abort();
  }
  if (st_thread_create(consumer, cons_buf, 0, 0) == NULL) {
    perror("Consumer thread not spawned: st_thread_create() has failed");
    abort();
  }

  st_thread_exit(NULL);
  free(s_buf_1);
  free(s_buf_2);
  free(s_buf_3);
  free(s_buf_4);
  free(hybrid_buf_a);
  free(hybrid_buf_b);
  free(hybrid_buf_c);
  free(prod_buf);
  free(cons_buf);
  free(print_buffer);

  return 0;
}
