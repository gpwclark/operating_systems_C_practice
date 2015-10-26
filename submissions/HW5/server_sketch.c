// I HAVE NEITHER GIVEN NOR RECEIVED HELP ON THIS PROGRAM - George Clark 720063665

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TMP_FILE_PREFIX_LEN 3
#define RESPONSE_STR_LEN 280
#define ERROR_STR_LEN 140
#define READ_BUF_SIZE 10
#define MAX_PID_LEN 5
#define MAX_ARGS 100
#define NUM_ARGS 6

// TODO obviously, the error printing needs to turn into a send
// TODO remove NUM_ARGS
// TODO implement the socket
// TODO free memory
// TODO check for length of max pid!!
// TODO COMMENT YOUR CODE and check that thing about loops and invariants and stuff.
// TODO are we checking for ALL of the errors? from all of the functions we use?


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

int run_command(char* cmd_to_run, char* tmp_file_name){
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
      sprintf(error_string, "Child's malloc failed for constructing the argument list: %s", strerror(errno));
      error_found = true;
      abort();
    }

    int ret_val = parse_args(cmd_to_run, string_array); 
    if(ret_val == -1){
      //ERROR, LINE EXCEEDS MAX LENGTH
      sprintf(error_string, "%s%d", "Line exceeds max number of arguments: ", MAX_ARGS - 1);
      error_found = true;
      abort();
    }

    errno = 0;
    int exec_return = execvp(*string_array, string_array);
    if (exec_return < 0) {
      sprintf(error_string, "Failed to execute, error with command as inputted: %s\n", strerror(errno));
      error_found = true;
      abort();
    }

    free(string_array);

  } else {
    //we are the parent
    //printf("I am the parent. My child's PID is %ld\n", (long)child_PID);

    //TODO comment in video about WNOHANG?
    errno = 0; 
    child_PID = waitpid(child_PID, &status, WCONTINUED);
    if (child_PID == -1){ //Wait for child process.  
      sprintf(error_string, "Wait error: %s\n", strerror(errno)); 
      error_found = true;
    } else { 
      /*
         if (WIFSIGNALED(status) != 0){
         printf("%s\n","Child process terminated because of receipt of signal.");
         } else if (WIFEXITED(status) != 0){
        // printf("%s\n", "Child process terminated as expected.");
        } else if(WIFSTOPPED(status != 0)){
        printf("%s\n", "Child process stopped.");
        } else{
        printf("%s\n", "Child process terminated with error.");
        }
      */
      int wif_exited = WIFEXITED(status);
      int exit_status;
      if (wif_exited != 0){
        exit_status = WEXITSTATUS(status); 
      }

      //TODO Need to use fopen() and read() on tmp_file_name to read what came 
      //in on stdout, as of now, unsure how this works IF there was some sort
      //of error, also, need to overwrite the file so we don't report old stuff?
      //We also have to send that response line...
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
          do {
            num_bytes_read = read(fd, buf, READ_BUF_SIZE);

            if (num_bytes_read == -1){
              if (errno != 0){
                sprintf(error_string, "Error while attempting to read file with stdout from exec comand: %s", strerror(errno));
                error_found = true;
                //TODO send error, file IO
              } else {
                //EOF reached
                break;
              }
            }

            for(int i = 0; i < num_bytes_read; i++) {
              fprintf(stderr, "%c", buf[i]);
              //TODO this gets sent to the client
              //we sort of want to put it in a buffer though, just in case 
              //clearing the file fails.
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
        //TODO
        //response needs wifexited and wexitstatus,
        //response is just RESPONSE: error_string wifexited and wexitstatus.
        //This will be a generic response shared by fatal error
        sprintf(response_string, "%s: %s\n", "RESPONSE", error_string);
        fprintf(stderr, "%s", response_string);
      } else {
        // then we send response 
        // response needs wifexited and wexitstatus
        // RESPONSE: End of Comand's stdout: exit_code: %d, exit_status: %d
        //TODO send normal response
        if (wif_exited != 0){
          sprintf(response_string, "%s: %s, Exit Code: %d, Exit Status: %d\n", "RESPONSE", "End of stdout", wif_exited, exit_status);
        } else {
          sprintf(response_string, "%s: %s, Exit Code: %d\n", "RESPONSE", "End of stdout", wif_exited);
        }
        fprintf(stderr, "%s", response_string);
      }

    }
    //printf("%s\n","Parent process ended.");

  } //end parent

  //TODO we could return -1 on error but it might not make sense since 
  //regardless of error we have already notified the client.
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

int main(int argc, char **argv) {
  char error_string[140];
  bool error_found = false;
  FILE *fp;
  //  char[] ps_string = "";
  //  char[] cmd_string = "";

  // program needs to accept a connection which it will use to get data line by
  // line, for this test server, we will pass it a filename where each line
  // in the file will be a command to be executed.
  // conn = socket();

  //TODO free this
  char **cmd_array = accept_connection();
  //TODO delete this BS code
  printf("returned string array:\n");
  int i = 0;
  while (i < NUM_ARGS){
    printf("%s", cmd_array[i]);
    ++i;
  }

  //Pattern: command, if(failure)(do_this) else (do_that)

  // first command
  //TODO free this
  char *tmp_file_name = get_tmp_file_name(); //tmpxxxx
  if (tmp_file_name == NULL){
    //TODO send error straight to client rather than printing.
    sprintf(error_string, "Failed to construct tmp_file_name_string\n");
    error_found = true;
  } else {
    //TODO we don't actually want to print this...
    printf("%s\n", tmp_file_name);

    // second command
    errno = 0;
    fp = freopen(tmp_file_name, "w+", stdout);
    if(fp == NULL){
      //TODO send error straight to client rather than printing.
      sprintf(error_string,"Failed to redirect stdout to tmp file: %s\n", strerror(errno));
      error_found = true;
    } else {

      // third command, will this be necessary when we implement socket?
      // something like string = conn.getLine();
      int i = 0;
      while (i < NUM_ARGS){
        // fourth command
        //TODO get actual error? depends on my hw2 program
        int ret_val = run_command(cmd_array[i], tmp_file_name);
        if (ret_val == -1){
          printf("the command did not work");
          //error = "command failed\n";
          //printf(error);
          //TODO the command failed, we pass the error to client via
          // what is written to stdout? or
          //TODO wouldn't the client handle what kind of error this is?
          //and sending it? Perhaps we should handle all of the sending
          //to the client in this main function and the client will just pass
          //the message directly or via some file?
        } else {

          /*
          //this is where we end up when everything went well
          //TODO this means here we need to read the tmp file
          //and send something to the client
          */
          ++i;
        }
      } // end while
    }
  } 

  if(error_found){
    //TODO
    //response is just RESPONSE: FATAL ERROR error_string.
    //This will be a generic response shared by normal error

  }
  if (fp != NULL){
    remove(tmp_file_name);
  }

  free(cmd_array);
  free(tmp_file_name);
}
