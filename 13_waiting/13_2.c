#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int pidfd_open(pid_t pid, unsigned int flags) {
  return (int)syscall(__NR_pidfd_open, pid, flags);
}

void proc_info(const char *procname) {
  printf("%s: PID %d, PPID %d, PGID %d, SID %d\n", procname, getpid(),
         getppid(), getpgid(0), getsid(0));
}

int main() {
  pid_t child_id = fork();
  if (child_id < 0) {
    perror("fork");
    return 1;
  }

  if (child_id == 0) {
    // This code is executed in child process only
    proc_info("Child");
    pid_t parent_pid = getppid();
    int pidfd = pidfd_open(parent_pid, 0);
    struct pollfd pollfd = {.fd = pidfd, .events = POLLIN};

    int poll_num = poll(&pollfd, 1, -1);
    if (poll_num < 0) {
      perror("poll");
      return -1;
    }

    printf("Parent process (PID = %d) has ended its execution\n", parent_pid);

    return 0;
  } else {
    // This code is executed in parent process only
    proc_info("Parent");
  }
  return 0;
}