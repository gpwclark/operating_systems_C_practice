#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/*
 * 1. Get familiar with errno
 * 2. ps -ef | egrep -e PID -e username
 * 3. then kill
 *
 *  a note on commenting, if a loop expects something to be formatted in a certain way
 *  then you have a pre-condition,
 *
 */

void childProcess(pid_t childPID){
    printf("I am the child. My childPID is %ld\n", (long)childPID);

}

int main() {
  errno = 0;
  pid_t childPID;
  childPID = fork();

  if (childPID < 0){
    printf("Error\n");

  }else if (childPID == 0){
    //we are the child
    childProcess(childPID);
    printf("Failed to with error %s\n",strerror(errno));

  }else {
    ////we are the parent
    //printf("I am the parent. My child's PID is %ld\n", (long)childPID);
    //errno = 0;
    //childPID = wait();

    //if (childPID < 0){
    //  printf("An error occurred while waiting: %s\n", strerror(errno));
    //} else{
    //  //check status
    //  // check man page for wait in order to understand how to do check status
    //}

    //printf("Done waiting\n");
  }
  
  printf("Exiting main\n");
  return 0;
}
