#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    perror("argc");
    return 1;
  }

  // Create queue or open existing one
  mqd_t queue_fd = mq_open(argv[1], O_WRONLY);
  if (queue_fd == (mqd_t)-1) {
    perror("mq_open");
    return 1;
  }

  // Send test message
  int result = 0;
  if (mq_send(queue_fd, argv[2], strlen(argv[2]), 0) < 0) {
    perror("Failed to send message");
    result = 1;
  }

  mq_close(queue_fd);
  return result;
}