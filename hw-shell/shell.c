#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;
int path_change = 0;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* token);
int cmd_cd(struct tokens* token);
int redirect(struct tokens* tokens);
char* path_resolve(char* f);
void pipe_executer2(struct tokens *tokens, int num_tasks);


/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_pwd, "pwd", "present working directory"},
    {cmd_cd, "cd", "change to directory"},
    {cmd_exit, "exit", "exit the command shell"},
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Present working directory. */
int cmd_pwd(struct tokens* tokens) {

  int size = 50;

  char* buffer = (char *) malloc(sizeof(char) * size);
  while (getcwd(buffer, size) == NULL){
    size *= 2;
  }
  
  printf("%s\n",buffer);
  free(buffer);

  return 0;
}

char* path_resolve(char* f){

  char* pathvar = getenv("PATH");
  char* p = strtok(pathvar, ":");
  char* copy = malloc(strlen(f) + 1);
  strcpy(copy, f);

  struct stat buffer;
  if(!stat(copy, &buffer)){
    return copy;
  }

  
  while(p){
    copy = malloc(strlen(p) + strlen(f) + 2);
    strcpy(copy, p);
    strcat(copy, "/");
    strcat(copy, f);

    if (!stat(copy, &buffer)){
      path_change = 1;
      break;
    }

    free(copy);
    copy = NULL;
    p = strtok(0, ":");
  }

  return copy;
}

/* Changes to directory. */

int cmd_cd(struct tokens *token) {
  char *new_dir = tokens_get_token(token, 1);

  if (chdir(new_dir)){
    printf("Error: Could not change to specified directory.\n");
    return 1;
  } else {
    return 0;
  }
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) { exit(0); }

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

/* Redirection. */

int redirect(struct tokens *tokens) {

  int len = tokens_get_length(tokens);
  int file;
  int file2;
  int j;
  char* source;

  for (int i = 0; i < len; i++){
    char* c = tokens_get_token(tokens, i);

    if (*c == '>'){

      char* source = tokens_get_token(tokens, i + 1);
      file2 = open(source, O_CREAT|O_TRUNC|O_WRONLY, 0644);
      
      dup2(file2, 1);

      j = i;

      while(j + 2 < len){
        tokens_set_token(tokens, j, j+2,'\0');
        j += 1;
      }

      len -= 2;
      i--;
      
    }

    if(*c == '<'){
      
      source = tokens_get_token(tokens, i + 1);
      file = open(source, O_RDONLY);
      dup2(file, 0);

      j = i;

      while(j + 2 < len){
        tokens_set_token(tokens, j, j+2,'\0');
        j += 1;
      }
    
      len -= 2;
      i--;


    }

    
  }

  return len;
}

void pipe_executer2(struct tokens *tokens, int num_tasks){

  int c;
  int input_index = 0;
  int output_index = 0;

  int fd[num_tasks - 1][2];
  for (c = 0; c < num_tasks - 1; c++){
    pipe(fd[c]);
  }

  int len = tokens_get_length(tokens);
  int i = 0;
  char* t[1024];
  int task_number = 0;
  pid_t pid;
  int status;
        
  for (int x = 0; x < len; x++) {
    char* s = tokens_get_token(tokens, x);
    t[x] = s;
  }

  t[len] = NULL;

  for (task_number = 0; task_number < num_tasks; task_number++){

    char* task[256];
    int k = 0;


    while(i < len && strcmp(t[i],"|") != 0){

      if(strcmp(t[i],"<") == 0){
        input_index = k;

      }

      if(strcmp(t[i],">") == 0){
        output_index = k;

      }

      task[k] = t[i];
      k++;
      i++;
    }

    task[k] = NULL;
    i++;

    pid = fork();

    if (pid == 0){

      if (task_number == 0){
        dup2(fd[task_number][1], 1);

        if (input_index){
          task[input_index] = NULL;
          int infile = open(task[input_index + 1], O_RDONLY);
          dup2(infile, 0);
  
        }
        
      }else if (task_number == (num_tasks - 1)){
          dup2(fd[task_number - 1][0], 0);

          if (output_index){

            task[output_index] = NULL;
            int outfile = open(task[output_index + 1], O_CREAT|O_TRUNC|O_WRONLY, 0666);
            dup2(outfile, 1);
          }

          

      } else {
        dup2(fd[task_number - 1][0], 0);
        dup2(fd[task_number][1], 1);
      }

      
  
      task[0] = path_resolve(task[0]);
      execv(task[0], task);

    } else {

      if (task_number == 0){
        close(fd[task_number][1]);
        
      } else if (task_number == num_tasks - 1){
        close(fd[task_number - 1][0]);

      } else {
        close(fd[task_number - 1][0]);
        close(fd[task_number][1]);
      }

      for (c = 0; c < num_tasks; c++){
        wait(&status);
      }
    }
  }

}

int main(unused int argc, unused char* argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;
  char* f;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens* tokens = tokenize(line);

    /* Find which built-in function to run. */
    f = tokens_get_token(tokens, 0);
    int fundex = lookup(f);


    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      int status;
      
      int len = tokens_get_length(tokens);
      int num_tasks = 1;

      for (int i = 0; i < len; i++) {
        char* s = tokens_get_token(tokens, i);
        if(strcmp(s, "|") == 0) {
          num_tasks += 1;
        }
      }

      pid_t cpid = fork();
      
      if (cpid == 0) {

        if (num_tasks > 1){
          pipe_executer2(tokens, num_tasks);
          exit(0);
        }

        len = redirect(tokens);

        char *t[256];

        for (int i = 0; i < len; i++) {
          char* s = tokens_get_token(tokens, i);
          t[i] = s;
        }

        t[len] = NULL;
        
        f = path_resolve(t[0]);

        if (f){
          if (execv(f, t)){
            perror("execv error, check args");
          }
        
        } else {
          perror("No such file or directory for args[0]");
          exit(1);
        }



      } else {
        wait(&status);
      }
      
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
