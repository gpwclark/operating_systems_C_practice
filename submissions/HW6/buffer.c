/* I have neither given nor recieved help on this assignment.
 * George Clark
 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "buffer.h"

#include <sys/mman.h>  
#include <unistd.h>    
#include <sys/types.h> 
#include <signal.h>    
#include <sys/stat.h>  
#include <fcntl.h>     

#define FILENAME "tmp_shared_mem_file"
#define TMP_FILE_STR_LEN 19
#define ERROR -1
#define SEMAPHORE_FULL_BUFFERS "/full_buffers"
#define FULL_BUFFERS_STR_LEN 13
#define SEMAPHORE_EMPTY_BUFFERS "/empty_buffers"
#define EMPTY_BUFFERS_STR_LEN 14

/* MAIN EXPLANATION
 * The buffer.c class is based on specs in the buffer.h file.
 * There is a buffer ADT that has a copy of the semaphores used by deposit
 * and remoove as well as a copy of the buffer that two threads share to
 * operate on deposit and remoove. The init function handles the set up
 * of the buffer ADT. The caller is responsible for freeing the memory
 * malloced by the init function.
 */


/* me define buffer size to be sufficiently large, if producer runs much more 
 * than the consumuer, we could have a situation where the producer is waiting
 * to write
 */
#define BUFFER_SIZE 200

/*
 * The deposit function in the producer - consumer paradigm uses these semaphore
 * values to keep track of how much data it has in it's "circular" buffer. Deposit
 * asserts that there are at least some buffers free before it inputs data, if
 * the buffer is full, it spins until it isn't. When it's done, it increments 
 * the fullBuffers semaphore indicating that there is now data in the buffer.
 */
void deposit(synced_buffer *s_buf, int value){
	//wait on emptyBuffers (equivalent to down() from the lectures)
	if (sem_wait(s_buf->sems->emptyBuffers) == ERROR) {
		perror("error while waiting on semaphore");
		exit(EXIT_FAILURE);
	}
  // assert that emptyBuffers != 0
  (s_buf->buffer[s_buf->nextIn]) = value;
  (s_buf->nextIn) = (s_buf->nextIn + 1) % BUFFER_SIZE;
  // assert buffer now has data
  
	//increment the value of charSem by calling sem_post
	//(equivalent to up() from the lectures)
	if (sem_post(s_buf->sems->fullBuffers) == ERROR) {
		perror("error incrementing semaphore");
		exit(EXIT_FAILURE);
	}
}

/*
 * The remoove function in the producer - consumer paradigm uses these semaphore
 * values to keep track of how much data it has in it's "circular" buffer. Remoove 
 * asserts that there are at least some buffers full before it removes data, if
 * the buffer is empty, it spins until it isn't. When it's done, it increments 
 * the emptyBuffers semaphore indicating that it remooved data from the buffer.
 */
int remoove(synced_buffer *s_buf){
	//wait on fullBuffers (equivalent to down() from the lectures)
	if (sem_wait(s_buf->sems->fullBuffers) == ERROR) {
		perror("error while waiting on semaphore");
		exit(EXIT_FAILURE);
	}
  // assert that fullBuffers != 0
  int data = (s_buf->buffer[s_buf->nextOut]);
  (s_buf->nextOut) = (s_buf->nextOut + 1) % BUFFER_SIZE;
  // assert buffer has had data removed

	//increment the value of charSem by calling sem_post
	//(equivalent to up() from the lectures)
	if (sem_post(s_buf->sems->emptyBuffers) == ERROR) {
		perror("error incrementing semaphore");
		exit(EXIT_FAILURE);
	}

  return data;
}

/*
 * This function is a clean up function for semaphores. After using
 * posix semaphores we have to close and unlink them. This function
 * is called by the main program after it is done using the buffer
 * to clear the system of all of the semaphores.
 */
void sem_cleanup(synced_buffer *s_buf){
	if (sem_close(s_buf->sems->emptyBuffers) == ERROR) {
		perror("error closing semaphore");
		exit(EXIT_FAILURE);
	}

	if (sem_close(s_buf->sems->fullBuffers) == ERROR) {
		perror("error closing semaphore");
		exit(EXIT_FAILURE);
	}
  
	//any semaphores we used must be unlinked
	if (sem_unlink(s_buf->sems->emptyBuffersName) == ERROR) {
		perror("error while unlinking semaphore");
		exit(EXIT_FAILURE);
	}

	if (sem_unlink(s_buf->sems->fullBuffersName) == ERROR) {
		perror("error while unlinking semaphore");
		exit(EXIT_FAILURE);
	}

}

/*
 * This is another clean up function called from the main program
 * to clear memory, in this case it is the shared memory.
 */
void delete_shared_mem(void* addr) {
	if (ERROR == munmap(addr, BUFFER_SIZE)) {
		perror("error deleting mmap");
		exit(EXIT_FAILURE);
	}
}


/*
 * This function creates the file that serves as shared memory between a producer-
 * consumer pair. It basically follows the conventions laid out in the video
 * with the caveat that I had to add in a mechanism to make each filename have
 * a different name as well as changing the size of the file.
 */
char* make_shared_mem(char* name, int name_length) {

  char *tmp_file = (char*)malloc(sizeof(char) * (TMP_FILE_STR_LEN + name_length + 1));
  sprintf(tmp_file,"%s%s",FILENAME,name);
  
	void *addr = 0;
	int protections = PROT_READ|PROT_WRITE;
	int flags = MAP_SHARED;
	int fd = open(tmp_file, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

	if (fd == ERROR) {
		perror("failed to open file");
		exit(EXIT_FAILURE);
	}

	if (lseek(fd, BUFFER_SIZE-1, SEEK_SET) == ERROR) {
		close(fd);
		perror("error calling lseek");
		exit(EXIT_FAILURE);
	}

	if (write(fd, "", BUFFER_SIZE) == ERROR) {
		close(fd);
		perror("error writing to file");
		exit(EXIT_FAILURE);
	}

	off_t offset = 0;
	
	char* mappedFile = mmap(addr, BUFFER_SIZE, protections, flags, fd, offset);
	
	if (( void *) ERROR == mappedFile) {
		perror("error with mmap");
		exit(EXIT_FAILURE);
	}

	return mappedFile;
}

/*
 * This function is responsible for instantiating a synced_buffer object.
 * Said object is simple. It has two semaphores, one reference to shared memory,
 * and two int variables to be used by the producer-consumer pair.
 */
synced_buffer *buffer_init(char* sem_name, int sem_name_length){
  
  char *emptyBuffersName = (char*)malloc(sizeof(char) * (EMPTY_BUFFERS_STR_LEN + sem_name_length + 1));
  sprintf(emptyBuffersName,"%s%s",SEMAPHORE_EMPTY_BUFFERS,sem_name);

  char *fullBuffersName = (char*)malloc(sizeof(char) * (FULL_BUFFERS_STR_LEN + sem_name_length + 1));
  sprintf(fullBuffersName,"%s%s",SEMAPHORE_FULL_BUFFERS,sem_name);
  
	sem_t *emptyBuffers = sem_open(emptyBuffersName, O_CREAT, S_IREAD | S_IWRITE, BUFFER_SIZE);

	if (emptyBuffers == SEM_FAILED) {
		perror("could not open semaphore");
		exit(EXIT_FAILURE);
	}

  
	sem_t *fullBuffers = sem_open(fullBuffersName, O_CREAT, S_IREAD | S_IWRITE, 0);

	if (fullBuffers == SEM_FAILED) {
		perror("could not open semaphore");
		exit(EXIT_FAILURE);
	}
  
  // Create the semaphore object for this particular buffer
  sem_conditionals *sems = (sem_conditionals*) malloc(sizeof(sem_conditionals));
  if(sems == NULL){
    //return null if malloc fails
    return NULL;
  }

  sems->emptyBuffers = emptyBuffers;
  sems->emptyBuffersName = emptyBuffersName;
  sems->fullBuffers = fullBuffers;
  sems->fullBuffersName = fullBuffersName;

  //me create buffer, and check for errrors.
	char *buffer = make_shared_mem(sem_name, sem_name_length);

  synced_buffer *sync_buf = (synced_buffer*) malloc(sizeof(synced_buffer));
  if(sync_buf == NULL){
    //return null if malloc fails
    return NULL;
  }
  
  sync_buf->buffer = buffer;
  sync_buf->sems = sems;
  sync_buf->nextIn = 0;
  sync_buf->nextOut = 0;
  
  
  return sync_buf;
}
