#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>
#define MAX_LINE 80 /* The maximum length command */
int main(void) {
  char *args[MAX_LINE / 2 + 1]; /* command line arguments */
  int should_run = 1;           /* flag to determine when to exit program */
  char *prev_args[MAX_LINE / 2 + 1];
  int size = MAX_LINE / 2 + 1;
  for (int i = 0; i < size; i++) {
    prev_args[i] = NULL;
  }

  int is_previous = 0;
  int is_previous_first = 1;
  while (should_run) {
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;
    printf("osh> ");
    fflush(stdout);

    /**
     * After reading user input, the steps are:
     * (1) fork a child process using fork()
     * (2) the child process will invoke execvp()
     * (3) parent will invoke wait() unless command included &
     */

    char buffer[MAX_LINE];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      printf("\nencountered EOF\n");
      break;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    char *token = strtok(buffer, " ");
    if (!token) {
      continue;
    }
    if (!strcmp(token, "exit")) {
      should_run = 0;
      break;
    }

    int run_in_background = 0;

    int index = 0;

    if (strcmp(token, "!!") == 0) {
      if (is_previous_first) {
        printf("No previous command found...\n");
        continue;
      }
      is_previous = 1;
      for (int i = 0; prev_args[i] != NULL; i++) {
        printf("%s ", prev_args[i]);
      }
      printf("\n");
    } else {

      while (token != NULL) {
        if (strcmp(token, "&") == 0) {
          printf("& encountered\n");
          run_in_background = 1;
        } else
          args[index++] = token;
        token = strtok(NULL, " ");
      }

      args[index] = NULL;
      is_previous = 0;
    }

    pid_t pid = fork();

    if (pid < 0) {
      printf("Can't create the child process\n");
      continue;
    } else if (pid == 0) {
      if (is_previous) {
        execvp(prev_args[0], prev_args);
      } else {
        execvp(args[0], args);
      }
      printf("execvp failed\n");
      exit(0);
    } else {

      if (!run_in_background)
        waitpid(pid, NULL, 0);
    }

    // if user type !! we shouldn't change the prev_args
    if (!is_previous) {
      int i;

      for (i = 0; prev_args[i] != NULL; i++) {
        free(prev_args[i]);
      }
      for (i = 0; args[i] != NULL; i++) {
        prev_args[i] = strdup(args[i]);
      }
      prev_args[i] = NULL;
    }

    is_previous_first = 0;

    // TODO -> free prev_args memory
  }
  return 0;
}
