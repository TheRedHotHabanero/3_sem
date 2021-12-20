#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <limits.h>

int main()
{
  const char *file_name = "counter.txt";
  int fd = 0;
  int counter = 1;

  // 10**20 <= 2**63 - 1 < 10**21
  char buf[20 + 1] = "";

  fd = open(file_name, O_RDWR | O_CREAT, 0600);
  // opening check
  if (fd == NULL)
  {
    printf ("Failed to open file");
    return -1;
  }

  // Locking file for reading and writing
  // On  Linux,  lockf()  is  just  an  interface  on top of fcntl(2) locking.
  lockf(fd, F_LOCK, 0); // F_LOCK Set an exclusive lock on the specified section of the file.

  long long int bytes_read = read(fd, buf, sizeof(buf));
  if (bytes_read < 0) {
    perror("read");
    lockf(fd, F_ULOCK, 0); // F_ULOCK Unlock the indicated section of the file
    close(fd);
    return -1;
  }

  char number[bytes_read > 0 ? (bytes_read + 1) : 2];
  memset(number, 0, sizeof(number));

  counter += atoi(buf);

  printf("The program worked %d times\n", counter);
  sprintf(number, "%d\n", counter);

  if (pwrite(fd, number, strlen(number), 0) < 0) {
    perror("write");
    lockf(fd, F_ULOCK, 0);
    close(fd);
    return -1;
  }

  lockf(fd, F_ULOCK, 0);
  close(fd);

  return 0;
}