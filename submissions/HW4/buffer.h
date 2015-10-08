#include <semaphore.h>
#include "sem_conditionals.h"

typedef struct {
  int *read_pipe;
  int *write_pipe;
} synced_buffer;

void deposit(synced_buffer *s_buf, int value);
int remoove(synced_buffer *s_buf);
synced_buffer *buffer_init();
