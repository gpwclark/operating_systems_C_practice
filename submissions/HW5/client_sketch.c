// I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

//TODO remove useless includes in server and client

int parse_response(){
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

  return 0;
}

int run_shell() {
//TODO need to decide when to get response and how to print it.
  static const char SHELL_PROMPT[] = "% ";
  char *line;
  size_t chars_to_read = 0;
  int chars_read;
  errno = 0; 

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
    if (errno < 0){
      perror("Failed to read input line");
      getline_failure = -1;
    }
    

    //TODO HERE we need to send the line to the server, but I think we also
    //need to make sure it isn't EOF. if it is send that to server for shutdown
    //but then break from this loop.
    //
  } while (chars_read != -1);
  free(line);
  //TODO close socket.

  return 0;
}

int main(int argc, char **argv) {
  int ret_val = 0;
  //TODO get host and port from argc and argv
  //TODO init the socket.

  ret_val = run_shell();
  //TODO close the socket?
  return ret_val;

}
