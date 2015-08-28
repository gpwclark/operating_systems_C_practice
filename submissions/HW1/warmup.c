/* I have neither given nor recieved help on this assignment.
 * Price Clark
 *
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

// Due to the special case in this program where there are two asterisks
// at the end of the line, one must look ahead before printing the string 
// to verify that it shouldn't be a character substitution instead.
int perform_special_char_substitutions(int *buffer){
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
  return 0;
}

// This handles the more complex 2:1 '**' -> '^' substitution.
int perform_complex_char_substitutions(int *buffer,int index,int character){

  if((index != 0) && (buffer[index-1] == '*') && (character == '*') ) {
    buffer[index - 1] = '^';
    --index;
  }

  return index;
}


// As of now there is only one direct 1:1 substitution should that change this
// function will accommodate said changes.
int perform_simple_char_substitutions(char character){
  if(character == '\n'){
    character = ' ';
  }
  return character;
}

// To abstract away what valid input is, should it change, this function checks
// to make sure the character from the input stream is something to process:
// printable characters or valid whitespace characters
bool is_valid_input(int character){
  if(isprint(character) || isspace(character)){
    return true;
  }else{
    return false;
  }
}


int main() {
  int input_char;

  int *buffer = (int *) malloc(sizeof(int)*LINE_LENGTH);
  if(buffer == NULL){
    return -1;
  }

  int count = 0;
  do{

    input_char = fgetc(stdin);

    if(is_valid_input(input_char)){
      input_char = perform_simple_char_substitutions(input_char);
      count = perform_complex_char_substitutions(buffer,count,input_char);

      if(count < LINE_LENGTH){
        buffer[count] = input_char;
      }

      ++count;

      if(count == LINE_LENGTH){

        if(perform_special_char_substitutions(buffer) != 0){
          return -1;
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
