#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  // Create pipe
  int pipe_fds[2]; // where we read from and where we write
  if (pipe(pipe_fds) < 0) {
    perror("failed to create pipe");
    return 1;
  }

  pid_t child_id = fork();
  if (child_id < 0) {
    perror("fork");
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return 1;
  }

  // In child prosess only
  if (child_id == 0) {
    // Close reading end of the pipe
    close(pipe_fds[0]);
    // Redirect stdout to pipe
    if (dup2(pipe_fds[1], fileno(stdout)) < 0) {
      perror("dup2");
      close(pipe_fds[1]);
      return 1;
    }
    close(pipe_fds[1]);

    execlp(
        // Filename to execute
        "last",
        // argv[0], argv[1], ...
        "last", NULL);
    // If it is, then something went wrong
    perror("execlp");
    return 1;
  }

  // In parent prosess only
  close(pipe_fds[1]); // We will not read anything

  // Redirect stdout to pipe
  if (dup2(pipe_fds[0], fileno(stdin)) < 0) {
    perror("dup2");
    close(pipe_fds[0]);
    return 1;
  }
  close(pipe_fds[0]);

  // Execute 'wc -l'
  execlp("wc", "-l", NULL);
  perror("failed to exec 'wc -l'");

  return 1;
}