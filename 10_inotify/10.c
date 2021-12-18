#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#define MEMBLOCK 4096

static ssize_t handle_events(int fd, int wd, const char *dir_name) {

  char buf[MEMBLOCK];
  const struct inotify_event *event;
  ssize_t len;
  char *ptr;

  // Loop while events can be read from inotify file descriptor
  while (1) {
    // Read events
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

      if (event->mask & IN_CREATE)
        printf("Created:  ");
      if (event->mask & IN_DELETE)
        printf("Deleted:  ");

      if (wd == event->wd)
        printf("%s/", dir_name);

      if (event->len)
        printf("%s", event->name);

      if (event->mask & IN_ISDIR)
        printf(" [directory]\n");
      else
        printf(" [file]\n");
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  const char *dir_name = ".";
  int fd = 0, wd = 0, poll_num = 0;

  nfds_t nfds = 2;
  struct pollfd fds[2];

  char input_buf[4096];

  // By default dir_name is current directory
  // If it is not the case, change dir_name
  if (argc == 2)
    dir_name = argv[1];

  printf("Press ENTER to break\n");

 // Monitoring filesystem events
  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    return -1;
  }

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

      // If creating or deleting event has happened
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
    return -1;
  }

  printf("===========   OVER   ===========\n");
  return 0;
}