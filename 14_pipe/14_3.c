#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  // create pipe
  int pipe_fds[2]; // откуда читаем и куда пишем
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

  // in child prosess only
  if (child_id == 0) {
    // close reading end of the pipe
    close(pipe_fds[0]);
    // redirect stdout to pipe
    if (dup2(pipe_fds[1], fileno(stdout)) < 0) {
      perror("dup2");
      close(pipe_fds[1]);
      return 1;
    }
    close(pipe_fds[1]);

    execlp(
        // filename to execute
        "last",
        // argv[0], argv[1], ...
        "last", NULL);
    //если выполняется, значит что то пошло не так
    perror("execlp");
    return 1;
  }

  //in parent prosess only
  close(pipe_fds[1]); // ничего не будем читать

  // redirect stdout to pipe
  if (dup2(pipe_fds[0], fileno(stdin)) < 0) {
    perror("dup2");
    close(pipe_fds[0]);
    return 1;
  }
  close(pipe_fds[0]);

  // execute 'wc -l'
  execlp("wc", "-l", NULL);
  perror("failed to exec 'wc -l'");

  return 1;
}