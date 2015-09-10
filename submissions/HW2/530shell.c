#include "CuTest.h"
#include "AllTests.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

//TODO make shell prompt a constant?
//TODO meaningful error messages
//TODO does shell_prompt print one extra time?
//TODO are sys wait.h/type.h AND unistd.h redundant?

char** parse_args(char *raw_arg_string){
  char** string_array;  

  return string_array; 
}

void test_run_shell(CuTest *tc) {
  int array_length;

  array_length = 6;
  char *input = strdup("oi i'm an argument, parse me");
  char **actual = parse_args(input); 
  char *expected[] = {"oi", "i'm", "an", "argument,", "parse", "me"};

  int i = 0;
  while(i < 6){
    CuAssertStrEquals(tc, expected[i], actual[i]);
    ++i;
  }
}



int run_shell() {
  char *shell_prompt = "% ";
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  pid_t child_PID;
  int status;
  errno = 0; 

  int input_char;
  //delete these when done experimenting 4
  int randomNumber, seed;
  char buffer[80];
  //end delete 

  do {
    printf("%s",shell_prompt);
    chars_read = getline(&line, &chars_to_read, stdin);

    if (chars_read != -1){
      puts(line);
    }else{
      if (errno != 0){
        perror("Failed to read input line");
      }
      break;
    }

    // Begin the forking
    child_PID = fork();

    if (child_PID < 0){
      // no child process created, fork failed
      perror("Process failed to fork");

      //fclose(stdout); ?? //TODO they do share streams?
      break;

    }else if (child_PID == 0){
      //we are the child

      printf("I am the child. My childPID is %ld\n", (long)child_PID);

      // delete this when done experimenting 17
      printf("Enter a random seed.n");
      if(gets(buffer)) {
        seed = atoi(buffer);
        srand(seed);
      }
      randomNumber = rand();        /* random end to child process */
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

    }else {
      //we are the parent
      printf("I am the parent. My child's PID is %ld\n", (long)child_PID);
      errno = 0; // TODO what do to with errno?

      child_PID = wait(&status);
      
      // delete this when done experimenting
      //
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
      printf("Parent process ended.n");

    }
  } while (input_char != EOF);
  free(line);

  if (errno == 0){
    return 0;
  }else{
    return -1;
  }
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
