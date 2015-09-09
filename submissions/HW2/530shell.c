#include "CuTest.h"
#include "AllTests.h"
#include "string.h"
#include "stdio.h"

//TODO make shell prompt a constant?

int runShell() {
  char *shell_prompt = "% ";
  printf("%s",shell_prompt);
  return 0;
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

  if ( (argc == 2) && (strcmp(argv[1], "--test") == 0) ){
    RunAllTests();
    
  }else {
    runShell();

  }

  return 0;
}
