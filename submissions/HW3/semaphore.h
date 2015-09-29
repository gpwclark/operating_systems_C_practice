#ifndef SEMAPHORE_GUARD
#define SEMAPHORE_GUARD

typedef struct 
{
  int value;
  st_cond_t sem_queue;
} semaphore;

void down(semaphore *s);
void up(semaphore *s);
void createSem(semaphore *s, int value);

#endif
