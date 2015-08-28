/*
 * Step 1. Read a character from stdin then print to stdout.
 * Step 2. Verify input to make sure that we only accept printable chars
 *         and the valid whitespace chars, tab, space, carriage return.
 * Step 3. Modify printing so that it only prints every LINE_LENGTH characters + newline
 * Step 4. Modify code to replace carriage return with a space
 * Step 5. Modify code to replace pair of asterisks with ^
 */ 

#include <stdint.h>                                                        
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define LINE_LENGTH 80

int main() {
  int input_char;
  bool asterisk_encountered = false;

  int *buffer = (int *) malloc(sizeof(int)*LINE_LENGTH);
  if(buffer == NULL){
    return -1;
  }
  
  int count = 0;
  do{

    input_char = getchar();

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
        }else{
          buffer[count] = input_char;
       }

      }
      
      ++count;

      if(count == LINE_LENGTH){
        for(int i = 0; i < LINE_LENGTH; ++i){
          fprintf(stdout, "%c", buffer[i]);
        }
        printf("\n");
        count = 0;
      }
  
    }

  }while(input_char != EOF);
  

  free(buffer);

  return 0;
}
