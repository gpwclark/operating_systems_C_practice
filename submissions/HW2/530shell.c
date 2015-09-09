#include "CuTest.h"
#include "AllTests.h"
#include "string.h"
#include "stdio.h"

char* StrToUpper(char* str) {
  return str;
}

void TestStrToUpper(CuTest *tc) {
  char* input = strdup("hello world");
  char* actual = StrToUpper(input);
  char* expected = "HELLO WORLD";
  CuAssertStrEquals(tc, expected, actual);
}

CuSuite* 530shellGetSuite() {
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, TestStrToUpper);
  return suite;
}

int main(int argc, char **argv) {
  if ( (argc > 1) && (strcmp(argv[1], "--test") == 0) ){
    RunAllTests();
  }else {
    printf("I'm the program.\n");
  }

  return 0;
}
