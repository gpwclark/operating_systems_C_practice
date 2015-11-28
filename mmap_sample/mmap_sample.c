//Ryan Doyle
//rpdoyle
//Heavily modified version of a file by Willem Wyndham
/*
 * This is a sample program for using memory mapping.
 * It is done by the function mmap (see man 2 mmap).
 * In this example there are two processes that use a memory mapped loction
 * to communicate. 
 * The first process simply reads from stdin and puts a char in the mapped file.
 * The second process reads from the mapped file and prints out the char.
 */
// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>  // mmap, munmap
#include <unistd.h>    // sleep
#include <sys/types.h> // kill
#include <signal.h>    // kill
#include <sys/stat.h>  // S_IREAD, S_IWRITE
#include <fcntl.h>     // O_CREAT
#include <semaphore.h> // sem_open, sem_close, etc.

#define FILESIZE sizeof(char)
#define FILENAME "tmpFileForMapping"
#define ERROR -1
#define SEMAPHORE_NAME "/charSem"

//prototypes for functions
void producer(char *mappedFile);
void consumer(char *mappedFile);
void waitForChildren(pid_t*);
pid_t forkChild(void (*func)(char *), char* mappedFile);
char* createMMAP();
void deleteMMAP(void*);

int main () {
	//get address of memory mapped location.
	char* mappedFile = createMMAP();
	
	//fork children
	pid_t childpids[2];
	childpids[0] = forkChild(producer, mappedFile);
	childpids[1] = forkChild(consumer, mappedFile);
 
	//wait for them
	waitForChildren(childpids);

	//any semaphores we used must be unlinked
	if(sem_unlink(SEMAPHORE_NAME) == ERROR) {
		perror("error while unlinking semaphore");
		exit(EXIT_FAILURE);
	}

	//cleanup
	deleteMMAP(mappedFile);
	exit(EXIT_SUCCESS);
}

char* createMMAP() {
	//these are the neccessary arguments for mmap. See man mmap.
	void* addr = 0;
	int protections = PROT_READ|PROT_WRITE; //can read and write
	int flags = MAP_SHARED; //it is shared between processes
	int fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE); //the file we wish to map to memory

	if (fd == ERROR) {
		perror("failed to open file");
		exit(EXIT_FAILURE);
	}

	//the calls in the next two if statements seek forward in the file and
	//write an empty string to the end of it to ensure the file
	//has the size that we expect when mapping it into memory

	if (lseek(fd, FILESIZE-1, SEEK_SET) == ERROR) {
		close(fd);
		perror("error calling lseek");
		exit(EXIT_FAILURE);
	}

	if (write(fd, "", 1) == ERROR) {
		close(fd);
		perror("error writing to file");
		exit(EXIT_FAILURE);
	}

	off_t offset = 0;
	
	//create memory map
	char* mappedFile = mmap(addr, FILESIZE, protections, flags, fd, offset);
	
	if (( void *) ERROR == mappedFile) { //on an error mmap returns void* -1.
		perror("error with mmap");
		exit(EXIT_FAILURE);
	}

	return mappedFile;
}

void deleteMMAP(void* addr) {
	//this deletes the memory map at given address. see man mmap
	if (ERROR == munmap(addr, FILESIZE)) {
		perror("error deleting mmap");
		exit(EXIT_FAILURE);
	}
}

pid_t forkChild(void (*function)(char *), char* mappedFile) {
	//this function takes a pointer to a function as an argument
	//and the functions argument. It then returns the forked child's pid.
	pid_t childpid;
	switch (childpid = fork()) {
		case ERROR:
			perror("fork error");
			exit(EXIT_FAILURE);
		case 0:	
			(*function)(mappedFile);
		default:
			return childpid;
	}
}

void waitForChildren(pid_t* childpids) {
	int status;
	while(ERROR < wait(&status)) { //here the parent waits on any child
		if(!WIFEXITED(status)){ //if the child terminated unsuccessfully, kill all children
			kill(childpids[0], SIGKILL);
	 		kill(childpids[1], SIGKILL);
			break;
	 	}
	}
}

//the execution path for producer
void producer(char* mappedFile) {
	//open the semaphore we need to use
	//SEMAPHORE_NAME is the name of the semaphore to open
	//O_CREAT specifies that the semaphore should be created if it does not already exist
	//S_IREAD | S_IWRITE gives us read and write permissions on the semaphore
	//0 specifies the initial value of the semaphore
	sem_t* charSem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IREAD | S_IWRITE, 0);

	if (charSem == SEM_FAILED) {
		perror("could not open semaphore");
		exit(EXIT_FAILURE);
	}

	//this process just reads a char and writes it to the memory mapped file
	char c = fgetc(stdin);

	mappedFile[0] = c;

	//increment the value of charSem by calling sem_post
	//(equivalent to up() from the lectures)
	if (sem_post(charSem) == ERROR) {
		perror("error incrementing semaphore");
		exit(EXIT_FAILURE);
	}

	//we need to close the semaphore we opened
	if (sem_close(charSem) == ERROR) {
		perror("error closing semaphore");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

//the execution path for consumer
void consumer(char* mappedFile) {
	//open the semaphore we need to use
	//SEMAPHORE_NAME is the name of the semaphore to open
	//O_CREAT specifies that the semaphore should be created if it does not already exist
	//S_IREAD | S_IWRITE gives us read and write permissions on the semaphore
	//0 specifies the initial value of the semaphore
	sem_t* charSem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IREAD | S_IWRITE, 0);

	if (charSem == SEM_FAILED) {
		perror("could not open semaphore");
		exit(EXIT_FAILURE);
	}

	//wait on charSem (equivalent to down() from the lectures)
	if (sem_wait(charSem) == ERROR) {
		perror("error while waiting on semaphore");
		exit(EXIT_FAILURE);
	}

	//this process reads a char from the memory mapped file and prints it
	char c = mappedFile[0];

	//we need to close the semaphore we opened
	if (sem_close(charSem) == ERROR) {
		perror("error closing semaphore");
		exit(EXIT_FAILURE);
	}

	//print char and a newline
	putchar(c);
	putchar('\n');

	exit(EXIT_SUCCESS);
}
