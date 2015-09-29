#ifndef THREAD_INST_GUARD
#define THREAD_INST_GUARD

#include "semaphore.h"
#include <stdio.h>

typedef struct {
    FILE *out_stream;
    semaphore *emptyBuffers;
    semaphore *fullBuffers;
    int *buffer;
}Thread_inst;

#endif
