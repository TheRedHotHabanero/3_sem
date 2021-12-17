#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Usage: %s /queuename\n", argv[0]);
    return 1;
  }

  // Create a new queue or open existing one
  mqd_t queue = mq_open(argv[1], O_RDWR | O_CREAT, 0600, NULL);

  if (queue == (mqd_t)-1) {
    perror("mq_open");
    return 1;
  }

  // Send test messages
  if (mq_send(queue, "ЫЫЫЫЫЫЫЫ", strlen("ЫЫЫЫЫЫЫЫ"), 0) == -1)
    perror("mq_send");

  // Get queue info and display it
  struct mq_attr queue_info = {};
  mq_getattr(queue, &queue_info);
  printf("Flags:                          %ld\n", queue_info.mq_flags);
  printf("Max number of messages:         %ld\n", queue_info.mq_maxmsg);
  printf("Max size of message:            %ld\n", queue_info.mq_msgsize);
  printf("Number of messages in queue:    %ld\n", queue_info.mq_curmsgs);

  // Clean up
  mq_unlink(argv[1]);
  mq_close(queue);

  return 0;
}