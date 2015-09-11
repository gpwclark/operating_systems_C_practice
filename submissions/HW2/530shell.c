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
//TODO delete the code from web and make sure mine works.
//TODO to more testing on parse_args with testing function including manually adding weird shit.

int parse_args(char *raw_arg_string, char **string_array){
  char *token;
  char white_space_delim[] = " \t\n\v\f\r";
  char *new_string = strdup(raw_arg_string);
  token = strtok(new_string, white_space_delim);

  int i = 0;
  while( (token != NULL) && (i < MAX_ARGS)){ 
    string_array[i] = token; 
    //printf("%s\n", string_array[i]);
    token = strtok(NULL, white_space_delim);
    ++i;
  }

  if(i < 100){
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
    printf("%s\n.", "malloc failed");
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
  //char *shell_prompt = "% ";
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  pid_t child_PID;
  int status;
  errno = 0; 

  int input_char;
  //delete these when done experimenting 4
  //int randomNumber, seed;  //DELETE ME
  //char buffer[80]; //DELETE ME
  //end delete 

  do {
    printf("%s",SHELL_PROMPT);

    errno = 0;
    chars_read = getline(&line, &chars_to_read, stdin);

    if (errno < 0){
      perror("Failed to read input line");
    }
    // Begin the forking

    errno = 0;
    child_PID = fork();

    if (child_PID < 0){
      // no child process created, fork failed
      perror("No child process, failed to fork");

    }else if (child_PID == 0){
      printf("I am the child. My childPID is %ld\n", (long)child_PID);
      errno = 0;
      char** string_array = (char**) malloc (MAX_ARGS * sizeof(char*));
      if(string_array == NULL){
        //TODO error message here!!!!
        // malloc sets errno!
        perror("Child's malloc failed for string_array");
        abort();
      }

      int ret_val = parse_args(line, string_array); 
      if(ret_val == -1){
        //TODO ERROR, LINE EXCEEDS MAX LENGTH
        //custom errors?
        printf("%s\n.","Line exceeds max number of arguments: 100");
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
      /*// delete this when done experimenting 17
        printf("Enter a random seed.n");
        if(gets(buffer)) {
        seed = atoi(buffer);
        srand(seed);
        }
        randomNumber = rand();
        if ((randomNumber % 2) == 0) {
        printf("Child process aborted; pid = %d.n", (int) child_PID);
        abort();
        fclose(stdout);
        }
        else {
        printf("Child process ended normally; pid = %d.n", (int) child_PID);
        exit(EXIT_SUCCESS);
        }
      //end delete
      */

      free(string_array);

    }else {
      //we are the parent
      printf("I am the parent. My child's PID is %ld\n", (long)child_PID);
      
      errno = 0; 
      child_PID = wait(&status);
      // delete this when done experimenting
      if (child_PID == -1){ //Wait for child process.
        perror("wait error");

      }  else { // Check Status 
        if (WIFSIGNALED(status) != 0)
          printf("Child process ended because of signal %d.n",
              WTERMSIG(status));
        else if (WIFEXITED(status) != 0)
          printf("Child process ended normally; status = %d.n",
              WEXITSTATUS(status));
        else
          printf("Child process did not end normally.n");
      }
      //END DELETE
      printf("Parent process ended.n");

    }
  } while (input_char != EOF);
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
