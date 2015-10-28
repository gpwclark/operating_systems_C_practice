// I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "Socket.h"
#include "Shell.h"

#define TMP_FILE_PREFIX_LEN 3
#define RESPONSE_STR_LEN 280
#define ERROR_STR_LEN 140
#define READ_BUF_SIZE 10
#define MAX_PID_LEN 5
#define MAX_ARGS 100

// TODO ALL THIS APPLIES TO THE CLIENT TOO
// TODO check for length of max pid!!
// TODO remove print statements
// TODO COMMENT YOUR CODE and check that thing about loops and invariants and stuff.
// TODO are we checking for ALL of the errors? from all of the functions we use?
// TODO make sure tmp file is removes on unnatural exit


//TODO if server strings are null terminated client should behave as expected.
int parse_args(char *raw_arg_string, char **string_array){
  char *token;
  char white_space_delim[] = " \t\n\v\f\r";
  char *new_string = strdup(raw_arg_string);
  token = strtok(new_string, white_space_delim);

  int i = 0;
  // Loop terminates when the token strtok parses evaluates to NULL,
  // indicating that the line of input is done parsing.
  while( (token != NULL) && (i < MAX_ARGS)){ 
    string_array[i] = token; 
    token = strtok(NULL, white_space_delim);
    ++i;
  }

  if(i < MAX_ARGS){
    return 0;
  }else{
    return -1;
  }
}

int run_command(char* cmd_to_run, char* tmp_file_name, Socket connection_socket){
  pid_t child_PID;
  int status;
  errno = 0;
  child_PID = fork();
  char error_string[ERROR_STR_LEN];
  bool error_found = false;
  //TODO make sure final RESPONSE line has everything we need.

  if (child_PID < 0){
    // no child process created, fork failed
    sprintf(error_string, "No child process, failed to fork: %s", strerror(errno));
    error_found = true;
  } else if (child_PID == 0){
    // printf("I am the child. My childPID is %ld\n", (long)child_PID);
    errno = 0;
    char** string_array = (char**) malloc (MAX_ARGS * sizeof(char*));
    if(string_array == NULL){
      free(string_array);
      exit(2);
    }

    int ret_val = parse_args(cmd_to_run, string_array); 
    if(ret_val == -1){
      //ERROR, LINE EXCEEDS MAX LENGTH
      free(string_array);
      exit(3);
    }

    errno = 0;
    int exec_return = execvp(*string_array, string_array);
    if (exec_return < 0) {
      fprintf(stderr, "EXEC FAILED\n");
      free(string_array);
      exit(4);
    }

    free(string_array);

  } else {
    //we are the parent
    //printf("I am the parent. My child's PID is %ld\n", (long)child_PID);

    //TODO comment in video about WNOHANG?
    errno = 0; 
    child_PID = wait(&status);
    //child_PID = waitpid(child_PID, &status, WCONTINUED);

    if (child_PID == -1){ //Wait for child process.  
      sprintf(error_string, "Wait error: %s\n", strerror(errno)); 
      error_found = true;
    } else { 

      int wif_exited = WIFEXITED(status);
      int exit_status;
      if (WIFSIGNALED(status) != 0){
        fprintf(stderr, "%s\n","Child process terminated because of receipt of signal.");
      } else if (WIFEXITED(status) != 0){
        fprintf(stderr, "%s\n", "Child process terminated as expected.");
        exit_status = WEXITSTATUS(status); 
      } else if(WIFSTOPPED(status != 0)){
        fprintf(stderr, "%s\n", "Child process stopped.");
      } else{
        fprintf(stderr, "%s\n", "Child process terminated with error.");
      }
      /*
         int wif_exited = WIFEXITED(status);
         fprintf(stderr, "%d exit_code\n", wif_exited);
         int exit_status;
         if (wif_exited != 0){
         exit_status = WEXITSTATUS(status); 
         }
         */

      //TODO We also have to send that response line for success
      //normal error, or fatal error.
      if(!error_found) {
        // Need to fopen and read tmp file. 
        // but also when we are done reading it we need to clear it so
        // we don't send duplicate data next time.
        FILE *fp;

        errno = 0;
        fp = fopen(tmp_file_name, "r");
        if (fp == NULL){
          sprintf(error_string, "Failed to open file with stdout for reading: %s", strerror(errno));
          error_found = true;
        } else {
          int fd = fileno(fp);
          char buf[READ_BUF_SIZE];
          int num_bytes_read;
          // successful reads
          // partial reads
          // EOF, ret val is -1 but errno is 0
          // IO error, ret val is -1 and errno is != 0

          errno = 0;
          int i;
          int sock_char;
          do {
            num_bytes_read = read(fd, buf, READ_BUF_SIZE);

            if (num_bytes_read == -1){
              if (errno != -1){
                sprintf(error_string, "Error while attempting to read file with stdout from exec comand: %s", strerror(errno));
                error_found = true;
                //TODO send error, file IO
                break;
              } else {
                //EOF reached
                break;
              }
            }
            int i;
            int sock_char;
            int sock_return;
            for(i = 0; i < num_bytes_read; i++) {
              fprintf(stderr, "%c", buf[i]);
              //TODO this gets sent to the client
              //we sort of want to put it in a buffer though, just in case 
              //clearing the file fails, or do we?
              //TODO we are supposed to use read but we are also supposed to send commands
              //line by line?
              sock_char = buf[i];
              sock_return = Socket_putc(sock_char, connection_socket); 
              if (sock_char == '\n'){
                sock_return = Socket_putc('\0', connection_socket); 
              }
              if (sock_return == EOF){
                printf("Sockets_putc returned EOF or error\n");
                Socket_close(connection_socket);
                exit (-1);
              }
            }
          } while (num_bytes_read != 0);

          FILE *fp = fopen(tmp_file_name, "w");
          if (fp == NULL){
            sprintf(error_string, "Failed to clear contents of tmp file: %s", strerror(errno));
            error_found = true;
          }
        }
      }

      char response_string[RESPONSE_STR_LEN];

      if(error_found){
        //TODO needs to be send to client
        sprintf(response_string, "%s: %s\n", RESPONSE, error_string);
        fprintf(stderr, "%s", response_string);
      } else {
        // RESPONSE: End of Comand's stdout: exit_code: %d, exit_status: %d
        if (wif_exited != 0){
          if (exit_status == 2){
            //exit code 2
            sprintf(response_string,"%s: %s, Exit Code: %d, Exit Status: %d\n", RESPONSE,"Error, Child's malloc failed for constructing the argument list", wif_exited, 1);
          } else if (exit_status ==3){
            //exit code 3
            sprintf(response_string, "%s: %s%d, Exit Code: %d, Exit Status: %d\n", RESPONSE,"Error, Line exceeds max number of arguments: ", MAX_ARGS - 1, wif_exited, 1);
          } else if (exit_status == 4){
            //Exit code 4
            sprintf(response_string, "%s: %s, Exit Code: %d, Exit Status: %d\n", RESPONSE, "Error, Failed to execute, error with command as inputted", wif_exited, 1);
          } else {
            sprintf(response_string, "%s: %s, Exit Code: %d, Exit Status: %d\n", RESPONSE, "End of stdout", wif_exited, exit_status);
          }
        } else {
          sprintf(response_string, "%s: %s, Exit Code: %d\n", RESPONSE, "End of stdout", wif_exited);
        }
        //TODO send normal response to client
        fprintf(stderr, "%s", response_string);
      }
      send_line(connection_socket, response_string);
    }
    //printf("%s\n","Parent process ended.");
  } //end parent

  return 0;
}

//TODO what about a call to getpid? error code for that right?
char* get_tmp_file_name(){
  char* tmp_file_name = (char*) malloc ((TMP_FILE_PREFIX_LEN + MAX_PID_LEN) * sizeof(char));
  int ret_val = sprintf(tmp_file_name, "tmp%d", getpid());

  if (ret_val < 0){
    return NULL;
  } else {
    return tmp_file_name;
  }
}

int send_line(Socket connection_socket, char* line){
  //TODO check if line==null
  int length = strlen(line);
  length = length + 1; /*need to include null terminator*/

  int i;
  int sock_char;
  int sock_return;
  for (i = 0; i < length; i++){
    sock_char = line[i];
    sock_return = Socket_putc(sock_char, connection_socket); 
    if (sock_return == EOF){
      printf("Sockets_putc returned EOF or error\n");
      Socket_close(connection_socket);
      exit (-1);
    }
  }

}

int main(int argc, char **argv) {
  char error_string[140];
  bool error_found = false;
  FILE *fp;
  int sock_char;
  int i;

  ServerSocket welcome_socket;
  Socket connection_socket;

  if (argc < 2) {
    printf("Error must specify port\n");
    return -1;
  }
  welcome_socket = ServerSocket_new(atoi(argv[1]));
  if (welcome_socket < 0){
    printf("Error failed to create welcome socket\n");
    return -1;
  }

  connection_socket = ServerSocket_accept(welcome_socket);
  if (connection_socket < 0){
    printf("Error, Server Socket accept failed");
    return -1;
  }

  // first command
  //TODO free this
  char *tmp_file_name = get_tmp_file_name(); //tmpxxxx
  if (tmp_file_name == NULL){
    sprintf(error_string, "Failed to construct tmp_file_name_string\n");
    error_found = true;
  } 
  // second command
  errno = 0;
  fp = freopen(tmp_file_name, "w+", stdout);
  if(fp == NULL){
    sprintf(error_string,"Failed to redirect stdout to tmp file: %s\n", strerror(errno));
    error_found = true;
  } 

  char* sock_array = (char*) malloc (MAX_LINE_LEN * sizeof(char));
  if(sock_array == NULL){
    sprintf(error_string, "Malloc failed for constructing receiving line");
    error_found = true;
  }

  if(error_found){
    char* response_string = (char*) malloc (RESPONSE_STR_LEN * sizeof(char));
    if(response_string == NULL){
      printf("Malloc failed for constructing response line");
    } else {
      sprintf(response_string, "%s: %s, %s\n", RESPONSE, FATAL_ERROR, error_string);
      fprintf(stderr, "%s", response_string);
      send_line(connection_socket, response_string);
      free(response_string);
    }
    //TODO send to client BUT MAKE SURE CLIENT TERMINATES.
  } else {

    //TODO main while loop terminates when sock_char == EOF
    //inner while loop waits for null character and then sends that
    //string to run command

    i = 0;
    do {  
      sock_char = Socket_getc(connection_socket);
      if (sock_char == EOF){
        printf("EOF received by client OR error");
        //TODO free memory here.
        break; 
      } else {
        sock_array[i] = sock_char;  
        i++;
        if (sock_char == '\0'){
          i = 0;
          run_command(sock_array, tmp_file_name, connection_socket);
        }
      }
    } while (sock_char != EOF);

  }

  if (fp != NULL){
    remove(tmp_file_name);
  }

  //TODO should welcome socket be closed earlier?
  free(sock_array);
  free(tmp_file_name);
  Socket_close(welcome_socket);
  Socket_close(connection_socket);
  return 0;
}
