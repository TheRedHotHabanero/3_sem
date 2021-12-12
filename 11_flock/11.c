#define _GNU_SOURCE
#define DIR_MODE 0777
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

int main(void)
{
  const char *file_name = "counter.txt";
  int fd = 0;
  int counter = 1;

  char buf[sizeof(unsigned long long int)];
  memset(buf, 0, sizeof(buf));

  fd = open(file_name, O_RDWR | O_CREAT, DIR_MODE);

  // Locking file for reading and writing
  lockf(fd, F_LOCK, 0);

  long long int bytes_read = read(fd, buf, sizeof(buf));
  if (bytes_read < 0) {
    perror("read");
    lockf(fd, F_ULOCK, 0);
    close(fd);
    return -1;
  }

  char number[bytes_read > 0 ? (bytes_read + 1) : 2];
  memset(number, 0, sizeof(number));

  counter += atoi(buf);

  printf("The program worked %d times\n", counter);
  sprintf(number, "%d\n", counter);

  if (pwrite(fd, number, sizeof(number), 0) < 0) {
    perror("write");
    lockf(fd, F_ULOCK, 0);
    close(fd);
    return -1;
  }

  lockf(fd, F_ULOCK, 0);
  close(fd);

  return 0;
}