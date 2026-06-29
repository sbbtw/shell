#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#define MAX_LINE 80 /* The maximum length command */
int main(void) {
  char *args[MAX_LINE / 2 + 1]; /* command line arguments */
  int should_run = 1;           /* flag to determine when to exit program */
  while (should_run) {
    printf("osh> ");
    fflush(stdout);

    /**
     * After reading user input, the steps are:
     * (1) fork a child process using fork()
     * (2) the child process will invoke execvp()
     * (3) parent will invoke wait() unless command included &
     */

    char buffer[1024];
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
    while (token != NULL) {
      if (strcmp(token, "&") == 0) {
        run_in_background = 1;
      } else
        args[index++] = token;
      token = strtok(NULL, " ");
    }

    args[index] = NULL;

    pid_t pid = fork();

    if (pid < 0) {
      printf("Can't create the child process\n");
      continue;
    } else if (pid == 0) {
      execvp(args[0], args);
      printf("execvp failed\n");
      exit(0);
    } else {
      if (!run_in_background)
        wait(NULL);
    }
  }
  return 0;
}
