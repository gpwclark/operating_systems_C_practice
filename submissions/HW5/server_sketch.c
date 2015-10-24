#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TMP_FILE_PREFIX_LEN 3
#define MAX_PID_LEN 5
#define MAX_ARGS 100
#define NUM_ARGS 6

// TODO obviously, the error printing needs to turn into a send
// TODO remove NUM_ARGS
// TODO implement the socket
// TODO free memory

char* get_tmp_file_name(){

  char* tmp_file_name = (char*) malloc ((TMP_FILE_PREFIX_LEN + MAX_PID_LEN) * sizeof(char));
  int ret_val = sprintf(tmp_file_name, "tmp%d", getpid());

  if (ret_val < 0){
    return NULL;
  } else {
    return tmp_file_name;
  }
}

char** accept_connection(){

  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char** string_array = (char**) malloc (NUM_ARGS * sizeof(char*));


  fp = fopen("cmd_file", "r");
  if (fp == NULL){
    return NULL;
  }

  int i = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    read = read + 1; //null terminator at end needs to be included

    char* string = (char*) malloc (read * sizeof(char));
    memcpy(string, line, read);
    string_array[i] = string;
    ++i;
  }

  fclose(fp);
  if (line){
    free(line);
  }

  return string_array;
}

int main(){
  char error[200];
  //  char[] ps_string = "";
  //  char[] cmd_string = "";

  // program needs to accept a connection which it will use to get data line by
  // line, for this test server, we will pass it a filename where each line
  // in the file will be a command to be executed.
  // conn = socket();

  char **cmd_array = accept_connection();
  printf("returned string array:\n");
  int i = 0;
  while (i < MAX_ARGS){
    printf("%s", cmd_array[i]);
    ++i;
  }

  //Pattern: command, if(failure)(do_this) else (do_that)

  // first command
  //TODO free this
  char *tmp_file_name = get_tmp_file_name(); //tmpxxxx
  if (tmp_file_name == NULL){
    sprintf(error, "Failed to construct tmp_file_name_string\n");
    printf(error);
  } else {
    printf("%s\n", tmp_file_name);

    // second command
    errno = 0;
    FILE *fp = freopen(tmp_file_name, "w+", stdout);
    if(fp == NULL){
      sprintf(error, "Failed to redirect stdout to tmp file: %s\n", strerror(errno));
      printf(error);
    } else {

      // third command, will this be necessary when we implement socket?
      // something like string = conn.getLine();
      int i = 0;
      while (i < NUM_ARGS){
        // fourth command
        //TODO get actual error? depends on my hw2 program
        int ret_val = run_command(cmd_string, fd, conn, ps_string);
        if (ret_val == -1){
          error = "command failed\n";
          printf(error);
        } else {
          /*
          //this is where we end up when everything went well

          }
          cmd_string = conn.get_line();
          if (cmd_string == ""){
          error = "failed to get string from connection\n";
          printf("%s\n", error);
          break;
          }
          */
      }
    } // end while
  }
} 

// TODO do not forget to remove the temp file
// if (fd != fail)
// remove(fd);
}
