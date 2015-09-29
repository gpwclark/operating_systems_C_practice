#include <semaphore.h>
#include "sem_conditionals.h"

typedef struct {
  int *buffer;
  sem_conditionals *sems;
} synced_buffer;

void deposit(synced_buffer *s_buf, int value);
int remoove(synced_buffer *s_buf);
synced_buffer *buffer_init();
