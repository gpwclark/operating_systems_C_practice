/*
 * Step 1. Read a character from stdin then print to stdout.
 * Step 2. Verify input to make sure that we only accept printable chars
 *         and the valid whitespace chars, tab, space, carriage return.
 * Step 3. Modify printing so that it only prints every 80 characters + newline
 * Step 4. Modify code to replace carriage return with a space
 * Step 5. Modify code to replace pair of asterisks with ^
 */ 

#include <stdint.h>                                                        
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
int main() {
  int inputChar;

 // while (inputChar != EOF){
    inputChar = getch(stdin);
   fprintf(stdout, "%c", inputChar);
  //}
    printf("meow");
  return 0;
}
