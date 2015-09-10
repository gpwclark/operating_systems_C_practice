#include "CuTest.h"
#include "AllTests.h"
#include "string.h"
#include "stdio.h"
#include "errno.h"

//TODO make shell prompt a constant?
//TODO meaningful error messages
//TODO does shell_prompt print one extra time?

int runShell() {
  char *shell_prompt = "% ";
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  errno = 0; 

  int input_char;

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
    
    

  } while (input_char != EOF);
  free(line);
  
  if (errno == 0){
    return 0;
  }else{
    return -1;
  }
}

void TestStrToUpper(CuTest *tc) {
  char* input = strdup("hello world");
  char* actual = "weee oh";
  char* expected = "HELLO WORLD";
  CuAssertStrEquals(tc, expected, actual);
}

CuSuite* ShellGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestStrToUpper);
  return suite;
}

int main(int argc, char **argv) {
  int ret_val = 0;

  if ( (argc == 2) && (strcmp(argv[1], "--test") == 0) ){
    RunAllTests();
    
  }else {
    ret_val = runShell();
  }

  return ret_val;
}
