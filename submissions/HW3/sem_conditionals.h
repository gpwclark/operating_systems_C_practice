#ifndef SEM_CONDITIONALS_GUARD 
#define SEM_CONDITIONALS_GUARD

#include "semaphore.h"
#include <stdio.h>

typedef struct {
    FILE *out_stream;
    semaphore *emptyBuffers;
    semaphore *fullBuffers;
}sem_conditionals;

#endif
