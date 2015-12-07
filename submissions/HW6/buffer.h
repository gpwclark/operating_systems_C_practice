#include "sem_conditionals.h"

typedef struct {
  char *buffer;
  sem_conditionals *sems;
  int nextIn;
  int nextOut;
} synced_buffer;

void deposit(synced_buffer *s_buf, int value);
int remoove(synced_buffer *s_buf);
char* make_shared_mem(char* name, int name_length);
void delete_shared_mem(void *addr); 
void sem_cleanup(synced_buffer *s_buf);
synced_buffer *buffer_init(char* sem_name, int sem_name_length);
