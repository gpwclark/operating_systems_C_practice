#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define NUM_ARGS 6

// TODO obviously, the error printing needs to turn into a send
// TODO free memory

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
  //What's up with C strings?
  //  char[] error = "";
  //  char[] ps_string = "";
  //  char[] cmd_string = "";

  // program needs to accept a connection which it will use to get data line by
  // line, for this test server, we will pass it a filename where each line
  // in the file will be a command to be executed.
  // conn = socket();

  char **cmd_array = accept_connection();
  printf("returned string array:\n");
  int i = 0;
  while (i < 6){
    printf("%s", cmd_array[i]);
    ++i;
  }

  //Pattern: command, if(failure)(do_this) else (do_that)

  // first command
  ps_string = get_ps_id_string(); //tmpxxxx
  if (ps_string == ""){
    error = "failed to construct ps_id_string\n";
    send(error);
  } else {
    printf(ps_string);

    /*
    // second command
    int fd = freopen(ps_string, 'w+',stdout);
    if(fd == fail){
    error = "failed to create tmp file for stdout\n";
    send(error);
    } else {

// third command
cmd_string = conn.get_line();
if (cmd_string == ""){
error = "failed to get string from connection\n";
send(error);
} else {
while (cmd_string != EOF) {

  // fourth command
  //TODO get actual error? depends on my hw2 program
  int ret_val = run_command(cmd_string, fd, conn, ps_string);
  if (ret_val == -1){
  error = "command failed\n";
  send(error);
  } else {
    //this is where we end up when everything went well

    }
    cmd_string = conn.get_line();
    if (cmd_string == ""){
    error = "failed to get string from connection\n";
    printf("%s\n", error);
    break;
    }
    } // end while
    }
    }
    */
  } 

// TODO do not forget to remove the temp file
// if (fd != fail)
// remove(fd);
}
