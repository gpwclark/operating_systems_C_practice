#include "CuTest.h"
#include "AllTests.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_ARGS 100

//TODO delete print statements and comments
//TODO test on classroom.cs.unc.edu
//TODO get rid of testing code

int parse_args(char *raw_arg_string, char **string_array){
  char *token;
  char white_space_delim[] = " \t\n\v\f\r";
  char *new_string = strdup(raw_arg_string);
  token = strtok(new_string, white_space_delim);

  int i = 0;
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

void test_run_shell(CuTest *tc) {
  int array_length;
  char **string_array;
  int ret_val;

  string_array = (char**)malloc(MAX_ARGS * sizeof(char*));
  if(string_array == NULL){
    printf("%s\n", "malloc failed.");
  }


  array_length = 11;
  char *input = "oi i'm an argument, parse me please" " look this is longer";
  ret_val = parse_args(input, string_array); 
  char *expected[] = {"oi", "i'm", "an", "argument,", "parse", "me", "please", "look", "this", "is", "longer"};
  int i = 0;
  printf("%s\n",*string_array);
  while((i < array_length) && (ret_val != -1)){
    CuAssertStrEquals(tc, expected[i], string_array[i]);
    ++i;
  }

  array_length = 13;
  input = "laksjdas dsaldkjsa dlsakjfal 0913  lkj l; jal sal s l			lk alks s";
  ret_val = parse_args(input, string_array);
  char *expected0[] = {"laksjdas", "dsaldkjsa", "dlsakjfal", "0913", "lkj", "l;", "jal", "sal", "s", "l", "lk", "alks", "s"};
  i = 0;
  while((i < array_length) && (ret_val != -1)){
    CuAssertStrEquals(tc, expected0[i], string_array[i]);
    ++i;
  }

  free(string_array);
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

  do {
    printf("%s",SHELL_PROMPT);

    errno = 0;
    chars_read = getline(&line, &chars_to_read, stdin);

    int getline_failure = 0;
    if (errno < 0){
      perror("Failed to read input line");
      getline_failure = -1;
    }
    // Begin the forking
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
          // I read you are supposed to close stdout if you use it in child,
          // please let me know if this is false.
          abort();

        }

        free(string_array);

      }else {
        //we are the parent
        //printf("I am the parent. My child's PID is %ld\n", (long)child_PID);

        errno = 0; 
        child_PID = wait(&status);
        // delete this when done experimenting
        if (child_PID == -1){ //Wait for child process.
          perror("wait error");

        } else{ 

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

CuSuite* ShellGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_run_shell);
  return suite;
}

int main(int argc, char **argv) {
  int ret_val = 0;

  if ( (argc == 2) && (strcmp(argv[1], "--test") == 0) ){
    RunAllTests();

  }else {
    ret_val = run_shell();
  }

  return ret_val;
}
