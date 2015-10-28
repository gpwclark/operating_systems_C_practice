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

// TODO remove print statements

/*
 * This is an identical function to the one from HW2. It uses strtok to break up
 * the command for execvp in the child process
 */
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

  /*
   * this main if else if, else handles the fork command.
   * if the fork failed we pass an erroe othewise we exec the command
   * and wait for it in the parent.
   */
  if (child_PID < 0){
    // no child process created, fork failed
    sprintf(error_string, "No child process, failed to fork: %s", strerror(errno));
    error_found = true;
  } else if (child_PID == 0){
    /*
     * This is the child process, if we get a specific error
     * we set a custom error code so we can pass the message to the 
     * client, if all goes well we simply exec the command
     * and it's output is captured in a tmp file we send to
     * the client.
     */
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

    /*
     * As per standing forking procedure, here we wait for the child process
     * to finish. If there is an error we note it and skip everything
     * that would occur if the parent behaved properly, otherwise
     * we get the WIFEXITED and if necessary WEXITSTATUS variables.
     */
    errno = 0; 
    child_PID = waitpid(child_PID, &status, WCONTINUED);

    if (child_PID == -1){ //Wait for child process.  
      sprintf(error_string, "Wait error: %s\n", strerror(errno)); 
      error_found = true;
    } else { 

      int wif_exited = WIFEXITED(status);
      int exit_status;
      fprintf(stderr, "%d exit_code\n", wif_exited);
      if (wif_exited != 0){
        exit_status = WEXITSTATUS(status); 
      }

      /*
       * If no error is found, we need to write the contents
       * of the tmp file (which are the output of the command from the
       * child process) to the client along with a RESPONSE message.
       */
      if(!error_found) {

        // Open the tmp file for reading and skip necessary logic if error occurs.
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
          int i;
          int sock_char;
          errno = 0;

          /*
           * This function uses the file we opened to read the contents
           * and pass them to the client over the socket. It exits when
           * read() reaches the end of the file.
           */
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

          /*
           * We need to clear the contents of the tmp file so they do not
           * get written to stdout the next time the run_command function
           * is called.
           */
          FILE *fp = fopen(tmp_file_name, "w");
          if (fp == NULL){
            sprintf(error_string, "Failed to clear contents of tmp file: %s", strerror(errno));
            error_found = true;
          }
        }
      }

      char response_string[RESPONSE_STR_LEN];

      /*
       * Once we have sent all of the contents of the tmp file
       * we need to send a response line, if an error occured 
       * during the sending of stdout, we handle that, otherwise
       * we send a normal RESPONSE.
       */
      if(error_found){
        //TODO needs to be send to client
        sprintf(response_string, "%s: %s\n", RESPONSE, error_string);
        fprintf(stderr, "%s", response_string);
      } else {
        /*
         * This logic handles sending the wif_exited value and if necessary
         * the exit status. The child function has custom error codes to 
         * ensure the error messages are informative. If there was a custom 
         * error code, we still pass the exit status as 1, because the actual
         * values are extraneous and 1 indicates that there was some sort of
         * error.
         */
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
      
      /*
       * This is where we actually send the response line, all the previous
       * logic has taken care of whether or not this is a normal response or 
       * an error so here we just pass whatever the response_string is to
       * send_line so that the client can receive the appropriate response.
       */
      send_line(connection_socket, response_string);
    }
    //printf("%s\n","Parent process ended.");
  } //end parent

  return 0;
}

/*
 * The server process redirects it stdout to a file. That file's name is
 * the string "tmp" concatenated with the PID of the current process. This function
 * uses sprintf to format that string so the file can be created.
 */
char* get_tmp_file_name(){
  char* tmp_file_name = (char*) malloc ((TMP_FILE_PREFIX_LEN + MAX_PID_LEN) * sizeof(char));
  int ret_val = sprintf(tmp_file_name, "tmp%d", getpid());

  if (ret_val < 0){
    return NULL;
  } else {
    return tmp_file_name;
  }
}

/* 
 * This function is used to pass RESPONSE lines to the client.
 * It simply takes a socket and a line and passes the line to
 * the client program.
 */
int send_line(Socket connection_socket, char* line){
  if(line == NULL){
    return -1;
  }
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
  return 0;
}

/*
 *  This is the server side implementation of a remote shell, it takes as its argument
 *  a port number correpsonding to the port number that the client is attempting
 *  to reach this server program on. This program accepts characters from the socket
 *  and when a full line has been passed takes the inputted line and executes it as a
 *  command. Other functions take care of passing the actual message and a response
 *  line back to the client program. More specifically this main function has to 
 *  establish a socket connection and do a few set up functions, if the connection
 *  is open but an error occurs that prevents the server from running, it lets
 *  the client now so it can terminate as well.
 */
int main(int argc, char **argv) {
  char error_string[140];
  bool error_found = false;
  FILE *fp;
  int sock_char;
  int i;

  ServerSocket welcome_socket;
  Socket connection_socket;

  // Verify correct number of arguments was passed in.
  if (argc < 2) {
    printf("Error must specify port\n");
    return -1;
  }

  /* 
   * Need to attain a welcome socket which is then passed to
   * another function to get the actual connection_socket
   * we need to communicate with the client.
   */
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

  // Get the name of the tmp file created for this specific instance
  // of this program.
  char *tmp_file_name = get_tmp_file_name(); //tmpxxxx
  if (tmp_file_name == NULL){
    sprintf(error_string, "Failed to construct tmp_file_name_string\n");
    error_found = true;
  } 
  /*
   * Use freopen to write stdout from exec'd command to the
   * file we opened in order to capture that output and
   * send it back to the client.
   */
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

  /*
   * If the server fails a malloc, fails to make the tmp file's name or the actual
   * temp file then the server can not function. In this case, we send a fatal
   * error to the client so it knows to shut down. Otherwise we execute the main 
   * loop of the program, to listen and accept characters until EOF.
   */
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
  } else {

    /*
     * This function listens on the socket and exits when it receives
     * EOF. In it's body, it checks to see if it receives the null terminator
     * if it does, then it sends the line that it was storing to the
     * run_command function which takes care of executing the command
     * and communicating the results with the client.
     */
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

  free(sock_array);
  free(tmp_file_name);
  Socket_close(welcome_socket);
  Socket_close(connection_socket);
  return 0;
}
