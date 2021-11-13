#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#define MEMBLOCK 4096

ssize_t write_all(int fd, const void *buf, size_t count)
{
  size_t bytes_written = 0;
  const uint8_t *buf_addr = (const uint8_t *)buf;
  while (bytes_written < count)
  {
    ssize_t res = write(fd, buf_addr + bytes_written, count - bytes_written);
    if (res < 0)
    { return res; }

    bytes_written += res;
  }
  return (ssize_t)bytes_written;
}

size_t copy_all(const int fd_1, const int fd_2)
{
  ssize_t copy_var = 1;
  while(copy_var > 0)
  {
    void* buf = calloc(MEMBLOCK, sizeof(char));
    assert(buf);
    copy_var = read(fd_1, buf, MEMBLOCK);
    if(copy_var < 0)
    {
      perror("Failed to read file");
      free(buf);
      return 7;
    }

    if(write_all(fd_2, buf, (size_t)copy_var) < 0)
    {
      perror("Failure to write");
      free(buf);
      return 8;
    }
    free(buf);
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s text\n", argv[0]);
    return 1;
  }

  struct stat sb = {};

  if (lstat(argv[1], &sb) == -1)
  {
    perror("lstat");
    return 2;
  }

  int fd_1 = open(argv[1], O_RDONLY);
  if (fd_1 == -1)
  {
    perror("Failed to open copied");
    return 3;
  }

  int fd_2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd_2 == -1)
  {
    perror("Failed to open file for writing");
    return 4;
  }

  copy_all(fd_1, fd_2);

  if (close(fd_1) < 0)
  {
    perror("Failure during close first");
    return 5;
  }
  if (close(fd_2) < 0)
  {
    perror("Failure during close second");
    return 6;
  }

  return 0;
}