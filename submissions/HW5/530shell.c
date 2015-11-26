#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_ARGS 100

/* I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665
 * This function takes as it's arguments the user inputted line as
 * raw_arg_string and the string_array which has been allocated to hold up to
 * MAX_ARGS. This function parses the line by whitespace and places the strings
 * in memory.
 */

int parse_args(char *raw_arg_string, char **string_array){
  char *token;
  char white_space_delim[] = " \t\n\v\f\r";
  char *new_string = strdup(raw_arg_string);
  token = strtok(new_string, white_space_delim);

  int i = 0;
  // Loop terminates when the token strtok parses evaluates to NULL,
  // indicating that the line of input is done parsing.
  while( (token != NULL) && (i < MAX_ARGS)){ 
    string_array[i] = token; 
    token = strtok(NULL, white_space_delim);
    ++i;
  }

  if(i < MAX_ARGS){
    return 0;
  }else{
    return -1;
  }
}

int run_shell() {
  static const char SHELL_PROMPT[] = "% ";
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  pid_t child_PID;
  int status;
  errno = 0; 

  int input_char;

  /*
   * This do while loop terminates when the return value for getline is -1, 
   * indicating that the getline function has encountered the EOF.
   */
  do {
    printf("%s",SHELL_PROMPT);

    errno = 0;
    chars_read = getline(&line, &chars_to_read, stdin);

    int getline_failure = 0;
    if (errno < 0){
      perror("Failed to read input line");
      getline_failure = -1;
    }
    /* 
     * Begin the forking, if the PID is less than 0 then fork failed, if it
     * is equivalent to 0 then running as child process, parse the input line 
     * and execvp the command, else running as parent, wait() for child
     * and report on status.
     */
    if(getline_failure == 0){

      errno = 0;
      child_PID = fork();

      if (child_PID < 0){
        // no child process created, fork failed
        perror("No child process, failed to fork");
      }else if (child_PID == 0){
        // printf("I am the child. My childPID is %ld\n", (long)child_PID);
        errno = 0;
        char** string_array = (char**) malloc (MAX_ARGS * sizeof(char*));
        if(string_array == NULL){
          perror("Child's malloc failed for constructing the argument list");
          abort();
        }

        int ret_val = parse_args(line, string_array); 
        if(ret_val == -1){
          //ERROR, LINE EXCEEDS MAX LENGTH
          printf("%s%d\n","Line exceeds max number of arguments: ", MAX_ARGS - 1);
          abort();
        }

        errno = 0;
        int exec_return = execvp(*string_array,string_array);
        if (exec_return < 0) {
          perror("Failed to execute, error with command as inputted");
          fclose(stdout); 
          abort();

        }

        free(string_array);

      }else {
        //we are the parent
        //printf("I am the parent. My child's PID is %ld\n", (long)child_PID);

        errno = 0; 
        child_PID = wait(&status);
        if (child_PID == -1){ //Wait for child process.
          perror("wait error");

        } else { 

          if (WIFSIGNALED(status) != 0){
            printf("%s\n","Child process terminated because of receipt of signal.");
          } else if (WIFEXITED(status) != 0){
            // printf("%s\n", "Child process terminated as expected.");
          } else if(WIFSTOPPED(status != 0)){
            printf("%s\n", "Child process stopped.");
          } else{
            printf("%s\n", "Child process terminated with error.");
          }

        }
        //printf("%s\n","Parent process ended.");

      }
    }
  } while (chars_read != -1);
  free(line);

  return 0;
}

/*
 * Main just serves to call run_shell, the function that sequentially reads lines, parses
 * them and then execvps the line if it is valid, terminating when EOF is reached.
 */
int main(int argc, char **argv) {
  int ret_val = 0;

  ret_val = run_shell();
  return ret_val;
}
