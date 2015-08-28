/*
 * Step 1. Read a character from stdin then print to stdout.
 * Step 2. Verify input to make sure that we only accept printable chars
 *         and the valid whitespace chars, tab, space, carriage return.
 * Step 3. Modify printing so that it only prints every LINE_LENGTH characters + newline
 * Step 4. Modify code to replace carriage return with a space
 * Step 5. Modify code to replace pair of asterisks with ^
 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LINE_LENGTH 10

int main() {
  int input_char;
  bool asterisk_encountered = false;

  int *buffer = (int *) malloc(sizeof(int)*LINE_LENGTH);
  if(buffer == NULL){
    return -1;
  }

  int count = 0;
  do{

    input_char = fgetc(stdin);

    if(isprint(input_char) || isspace(input_char)){

      if(input_char == '\n'){
        input_char = ' ';
      }
      if(input_char == '*'){
        asterisk_encountered = true;
      }

      if(count < LINE_LENGTH){
        if(asterisk_encountered && (count != 0) && (buffer[count-1] == '*') ){
          buffer[count-1] = '^'; 
          --count;
          asterisk_encountered = false;
        }

        else{
          buffer[count] = input_char;
        }

      }

      ++count;

      if(count == LINE_LENGTH){

        if(buffer[LINE_LENGTH-1] == '*'){
          int temp_char = fgetc(stdin);
          if(temp_char == '*'){
            buffer[LINE_LENGTH-1] = '^';
          }else{
            int ret_val = ungetc(temp_char,stdin);

            // If the return value is EOF then the stream is broken so I
            // free the buffer and exit with an error code.
            if(ret_val == EOF){
              free(buffer);
              return -1;
            }
          }
        }

        int i;
        for(i = 0; i < LINE_LENGTH; ++i){
          fprintf(stdout, "%c", buffer[i]);
        }
        printf("\n");
        count = 0;
        memset(buffer, 0, LINE_LENGTH*sizeof(int));
      }

    }

  }while(input_char != EOF);

  free(buffer);

  return 0;
}
