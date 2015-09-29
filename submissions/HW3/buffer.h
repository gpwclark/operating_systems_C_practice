#include <semaphore.h>
#include "Thread_inst.h"

void deposit(Thread_inst *thread, int value);
int remoove(Thread_inst *thread);
Thread_inst *buffer_init();
