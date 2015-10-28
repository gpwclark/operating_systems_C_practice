// I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#include "Socket.h"
#include "Shell.h"

/*
 * This program serves as the exit condition for the inner loop in run_shell.
 * Run_shell's main loop watches to see if EOF occurs, the inner loop makes sure
 * that we continue to read characters from the socket until the server has sent
 * a response, indicating that the server is done processing the command and the
 * client should pass it another command to run (providing there is one and EOF has
 * not been reached). This command needs to distiguish between two main cases. 
 * In the case that the line does not contain RESPONSE in all capital letters,
 * we just print the line and return true because a line containing the string 
 * "RESPONSE" indicates that stdout is done. If the line does contain "RESPONSE"
 * then we print the line and return false. In the case that a FATAL_ERROR has 
 * occurred we print the error and shut down. There are a few highly improbable
 * errors that could occur and would cause the server to shut down, in this
 * case we need a mechanism to shut down the client as well, this is what
 * a RESPONSE that is also a FATAL_ERROR triggers.
 */
bool parse_response(char* response_string){
  char* ret_val;
  ret_val = strstr(response_string, RESPONSE);
  printf("%s", response_string);
  if (ret_val == NULL) {
    return true;
  } else {
    ret_val = strstr(response_string, FATAL_ERROR);
    if (ret_val == NULL){
      return false;
    } else {
      printf("Fatal error: Server has terminated and client must terminate as a result\n");
      exit (-1);
    }
  }
}

/*
 * This function takes a socket and then prints a prompt. The function getline()
 * waits for input on stdin. 
 */
int run_shell(Socket connection_socket) {
  char* char_array = (char*) malloc (MAX_LINE_LEN * sizeof(char));
  static const char SHELL_PROMPT[] = "% ";
  size_t chars_to_read = 0;
  bool response_parse_ret;
  bool getline_error;
  char sock_ret_line[MAX_LINE_LEN];
  char *line;
  int chars_read;
  int sock_char;
  int sock_return;
  int input_char;
  int i = 0;
  errno = 0; 

  /*
   * This do while loop terminates when the return value for getline is -1, 
   * indicating that the getline function has encountered the EOF.
   */
  do {
    getline_error = false;
    printf("%s",SHELL_PROMPT);

    /*
     * First we receive a line on stdin and then we pass that line, including
     * the null terminator to the server program.
     */
    errno = 0;
    chars_read = getline(&line, &chars_to_read, stdin);

    /* If getline fails we need to print the error. And make sure
     * we don't do anything with the line this iteration of the loop.
    */
    if (errno != 0 ){
      perror("Failed to read input line");
      getline_error = true;
    } else {

      // Send EOF to server because getline returned EOF. 
      if (chars_read == -1){
        sock_char = EOF;
        sock_return = Socket_putc(sock_char, connection_socket); 
        if (sock_return == EOF){
          printf("Sockets_putc returned EOF or error\n");
          Socket_close(connection_socket);
        }
        break;
      }

    }


    chars_read = chars_read + 1; /* getline doesn't include null terminator*/
    if ( (chars_read > MAX_LINE_LEN) || (getline_error) ){
      getline_error = false;
      printf("Error, maximum line length is 1024");
    } else {

      /* 
       * This while loop reads through every character of the string inputted
       * from stdin including the null terminator and sends it to the server.
      */
      i = 0;
      while (i < chars_read){
        sock_char = line[i];
        sock_return = Socket_putc(sock_char, connection_socket); 
        if (sock_return == EOF){
          printf("Sockets_putc returned EOF or error\n");
          Socket_close(connection_socket);
          exit (-1);
        }
        i++;
      }


      /*
       * This while loop depends on an inner while loop to get a line of input
       * from the server, after this happens we copy the line to a string
       * we can use as a parameter and then give it to a function called
       * parse_response. Parse response returns true if there are more
       * lines of input (for the given command) that need to come back from the
       * server. When the parse_response function receives a response line, it
       * prints this line (just like the ones before it) but returns false. This
       * is the exit condition for this loop. When this occurs we go back to the 
       * top of the loop outside this one and continue to receive line from stdin.
       */
      do {
        /*
         * This inner while loop takes input from the server up to a predetermined
         * max character length. It's main exit condition is when the server passes
         * the null terminator, this indicates that a full line has been passed
         * to the client from the server. When this happens, we break from this
         * while loop and depend on the logic of the outer while loop to continue
         * executing as long as there are lines to receive from the server.
         */
        i = 0;
        while ( i < MAX_LINE_LEN){
          sock_char = Socket_getc(connection_socket);
          if (sock_char == EOF){
            printf("Sockets_putc returned EOF or error\n");
            Socket_close(connection_socket);
            exit (-1);
          } else {
            sock_ret_line[i] = sock_char;  
            if (sock_ret_line[i] == '\0'){
              break;
            }
          }
          i++;
        }
        /*verify string is null terminated*/
        if ( i == MAX_LINE_LEN){
          sock_ret_line[i-1] = '\0';
        }
        memcpy(char_array, &sock_ret_line, MAX_LINE_LEN);
        response_parse_ret = parse_response(char_array);

      } while (response_parse_ret != false); 
    }
  } while (chars_read != -1);

  free(line);
  free(char_array);
  return 0;
}

/*
 * This is the client side implementation of a remote shell, it takes as its
 * arguments a DNS hostname and a port number corresponding the the port number
 * that the server is listening on. Note the server must be started before the
 * client runs otherwise the program will fail. This program uses sockets to 
 * pass the commands inputted to the client to the server and the server
 * executes the command and passes what came to stdout back to the client
 * along with a repsonse line.
 */

int main(int argc, char **argv) {
  int ret_val = 0;
  const char *RESPONSE;
  const char *FATAL_ERROR;

  if (argc < 3){
    printf("Error, program needs DNS hostname and port number as arguments\n");
    return -1;
  }

  Socket connection_socket;
  connection_socket = Socket_new(argv[1], atoi(argv[2]));
  if (connection_socket < 0){
    printf("Error, could not connect to server\n");
    return -1;
  }

  /*
   * The function run_shell simply accepts commands until it receives EOF for
   * stdin, from the server (on error), or if other error occurs.
   */
  ret_val = run_shell(connection_socket);
  Socket_close(connection_socket);
  return ret_val;
}
