#ifndef SEM_CONDITIONALS_GUARD 
#define SEM_CONDITIONALS_GUARD

#include <semaphore.h>

typedef struct {
    sem_t *emptyBuffers;
    char *emptyBuffersName;
    sem_t *fullBuffers;
    char *fullBuffersName;
}sem_conditionals;

#endif
