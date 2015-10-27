// I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#include "Socket.h"
#include "Shell.h"
//TODO remove useless includes in server and client

bool parse_response(char* response_string){
  char* ret_val;
  //read_in_response.
  // Need to distinguish between 3 different cases.
  //   1. FATAL error, server can't do a thing so we ahve to abort
  //     entirely.
  //   2. and 3.
  //     now, because the server is writing all none fatal errors
  //     to the socket, and an error will just be one line (theoretically)
  //     and a normal response will be w/e cmd output is followed by
  //     a response line, it doesn't seem like I even need to freaking
  //     worry about differentiating between 2 and 3.
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

  return 0;
}

int run_shell(Socket connection_socket) {
  //TODO need to decide when to get response and how to print it.
  static const char SHELL_PROMPT[] = "% ";
  char* char_array = (char*) malloc (MAX_LINE_LEN * sizeof(char));
  char sock_ret_line[MAX_LINE_LEN];
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  errno = 0; 
  int sock_char;
  int sock_return;
  int i = 0;

  int input_char;

  /*
   * This do while loop terminates when the return value for getline is -1, 
   * indicating that the getline function has encountered the EOF.
   */
  do {
    printf("%s",SHELL_PROMPT);

    errno = 0;
    chars_read = getline(&line, &chars_to_read, stdin);
    int getline_failure = 0;
    if (errno != 0 ){
      perror("Failed to read input line");
      getline_failure = -1;
      break;
    }
    chars_read = chars_read + 1; /* getline doesn't include null terminator*/
    if (chars_read > MAX_LINE_LEN){
      printf("Error, maximum line length is 1024");
    } else {

      /* TODO comment
      */
      //TODO HERE we need to send the line to the server, but I think we also
      //need to make sure it isn't EOF. if it is send that to server for shutdown
      //but then break from this loop.
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


      //TODO after sending line to server, we wait for response. If the response is a
      //fatal error we need to show it AND exit. Otherwise, just show whatever it is.
      // use parse_response?
      do {
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
        // print return line
        // TODO check errors for malloc and memcpy
        memcpy(char_array, &sock_ret_line, MAX_LINE_LEN);

      } while (parse_response(char_array) != false); 
    }
  } while (chars_read != -1);

      free(line);
      free(char_array);
      return 0;
}

int main(int argc, char **argv) {
  int ret_val = 0;
  const char *RESPONSE;
  const char *FATAL_ERROR;
  //TODO get host and port from argc and argv
  //TODO init the socket.

  printf("that's true");
  if (argc < 3){
    printf("Error, program needs DNS hostname and port number as arguments\n");
    return -1;
  }
  printf("that's true too");

  Socket connection_socket;
  connection_socket = Socket_new(argv[1], atoi(argv[2]));
  if (connection_socket < 0){
    printf("Error, could not connect to server\n");
    return -1;
  }

  ret_val = run_shell(connection_socket);
  //TODO close the socket?
  Socket_close(connection_socket);
  return ret_val;
}
