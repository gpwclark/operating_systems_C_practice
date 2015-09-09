    #include "CuTest.h"
    #include "stdio.h"

    CuSuite* 530shellGetSuite();

    void RunAllTests(void) {
        CuString *output = CuStringNew();
        CuSuite* suite = CuSuiteNew();
        CuSuiteAddSuite(suite, 530shellGetSuite());
        CuSuiteRun(suite);
        CuSuiteSummary(suite, output);
        CuSuiteDetails(suite, output);
        printf("%s\n", output->buffer);
    }
