#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/types.h>
#define MEMBLOCK 4096


// The behavior of this variable cannot be optimized
volatile int g_last_signal = -1;
volatile siginfo_t *g_from_who;

const int signls[_NSIG] = {
    SIGABRT,   SIGALRM, SIGBUS,  SIGCHLD, SIGCLD,    SIGCONT, SIGFPE,
    SIGHUP,    SIGILL,  SIGINT,  SIGIO,   SIGIOT,    SIGPIPE, SIGPOLL,
    SIGPROF,   SIGPWR,  SIGQUIT, SIGSEGV, SIGSTKFLT, SIGTSTP, SIGSYS,
    SIGTERM,   SIGTRAP, SIGTTIN, SIGTTOU, SIGURG,    SIGUSR1, SIGUSR2,
    SIGVTALRM, SIGXCPU, SIGXFSZ, SIGWINCH};

void sig_handler(int signum, siginfo_t *info, void *ucontext) {
  (void)ucontext;
  g_last_signal = signum;
  g_from_who = info;
}

static ssize_t handle_events(int fd, int wd, const char *dir_name) {
  char buf[MEMBLOCK];
  const struct inotify_event *event;
  ssize_t len;
  char *ptr;

  // Loop while events can be read from inotify file descriptor
  while (1) {
    // Read some events
    len = read(fd, buf, sizeof(buf));
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      return -1;
    }

    // If the nonblocking read() found no events to read, then
    // it returns -1 with errno set to EAGAIN. In that case,
    // we exit the loop
    if (len <= 0)
      break;

    // Loop over all events in the buffer
    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;

      // Print event type
      if (event->mask & IN_CREATE)
        printf("Created:  ");
      if (event->mask & IN_DELETE)
        printf("Deleted:  ");

      // Print name of the directory
      if (wd == event->wd) {
        printf("%s/", dir_name);
      }

      // Print the name of the file
      if (event->len)
        printf("%s", event->name);

      // Print type of filesystem object
      if (event->mask & IN_ISDIR)
        printf(" [directory]\n");
      else
        printf(" [file]\n");
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  int result = 0;

  const char *dir_name = ".";
  int fd = 0, wd = 0, poll_num = 0;

  nfds_t nfds = 2;
  struct pollfd fds[2];

  char input_buf[MEMBLOCK];

  struct sigaction recieved = {};

  recieved.sa_flags = SA_SIGINFO;
  recieved.sa_sigaction = sig_handler;

  for (int i = 0; i < _NSIG; i++) {
    if (sigaction(signls[i], &recieved, NULL) < 0) {
      perror("sigaction");
      result = -1;
    }
  }

  // By default dir_name is current directory
  // if it is not the case, change dir_name
  if (argc == 2)
    dir_name = argv[1];

  printf("Press ENTER key to terminate.\n");

  // Initializing inotify
  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    return -1;
  }

  // Adding watch
  wd = inotify_add_watch(fd, dir_name, IN_CREATE | IN_DELETE);
  if (wd == -1) {
    fprintf(stderr, "Cannot watch '%s': %s\n", dir_name, strerror(errno));
    return -1;
  }

  // Setting variables for poll
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  // Reading events in a loop
  while (1) {
    // Checking if a signal was sent
    if (g_last_signal != -1) {
      printf("The signal %d (%s) was sent from PID=%d\n", g_last_signal,
             strsignal(g_last_signal), g_from_who->si_pid);

      if (g_last_signal == SIGTERM) {
        printf("Terminating...\n");
        return 0;
      }

      if (g_last_signal == SIGQUIT) {
        printf("Exiting...\n");
        return 0;
      }

      g_last_signal = -1;
    }

    // Checking for events
    poll_num = poll(fds, nfds, -1);

    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      return -1;
    }

    // If an event
    if (poll_num > 0) {
      // If entered something
      if (fds[0].revents & POLLIN) {
        if (read(STDIN_FILENO, input_buf, sizeof(input_buf)) > 0 &&
            input_buf[0] == '\n') {
          printf("Stop listening for events?(Y/N)\n");

          if (read(STDIN_FILENO, input_buf, sizeof(input_buf)) < 0) {
            perror("read");
            return -1;
          }

          if (input_buf[0] == 'Y' || input_buf[0] == 'y')
            break;
          else
            continue;
        }
      }

      // If event has happened
      if (fds[1].revents & POLLIN)
        if (handle_events(fd, wd, dir_name) < 0) {
          perror("handle_events");
          return -1;
        }
    }
  }

  // Closing descriptor inotify returned
  if (close(fd) < 0) {
    perror("close");
    result = -1;
  }

  printf("Listening for events stopped.\n");

  return result;
}