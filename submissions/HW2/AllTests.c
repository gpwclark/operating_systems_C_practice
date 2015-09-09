    #include "CuTest.h"
    #include "stdio.h"

    CuSuite* ShellGetSuite();

    void RunAllTests(void) {
        CuString *output = CuStringNew();
        CuSuite* suite = CuSuiteNew();
        CuSuiteAddSuite(suite, ShellGetSuite());
        CuSuiteRun(suite);
        CuSuiteSummary(suite, output);
        CuSuiteDetails(suite, output);
        printf("%s\n", output->buffer);
    }
