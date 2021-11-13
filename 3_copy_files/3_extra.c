#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#define MEMBLOCK 4096

ssize_t p_write_all(int fd, const void *buf, size_t count, long int counter)
{
  size_t bytes_written = 0;
  const uint8_t *buf_addr = (const uint8_t *)buf;
  while (bytes_written < count)
  {
    ssize_t res = pwrite(fd, buf_addr + bytes_written, count - bytes_written, counter);
    if (res < 0)
    { return res; }

    bytes_written += res;
    counter += res;
  }
  return (ssize_t)bytes_written;
}

size_t copy_all(const int fd_1, const int fd_2)
{
  off_t counter = 0; // off_t is like size_t but for files
  ssize_t copy_var = 1;
  while(copy_var > 0)
  {
    void* buf = calloc(MEMBLOCK, sizeof(char));
    assert(buf);
    copy_var = pread(fd_1, buf, MEMBLOCK, counter);
    if(copy_var < 0)
    {
      perror("Failed to read file");
      free(buf);
      return 7;
    }

    if(p_write_all(fd_2, buf, (size_t)copy_var, counter) < 0)
    {
      perror("Failure to write");
      free(buf);
      return 8;
    }
    counter += copy_var;
    free(buf);
  }
  return 0;
}

size_t create_symlink(const char* path, const char* name)
{
  char* buf = (char*)calloc(PATH_MAX, sizeof(char));
  size_t res = 0;

  if (readlink(path, buf, PATH_MAX) < 0)
  {
    fprintf(stderr, "Failed while reading link\n");
    res = 10;
  }
  if (symlink(buf, name) < 0)
  {
    fprintf(stderr, "Failed to create the linl\n");
    res = 11;
  }
  free(buf);
    return res;
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

 // macros from man fstat
  switch(sb.st_mode & S_IFMT)
  {
    case S_IFREG:
    {
      int fd_1 = open(argv[1], O_RDONLY);
      if (fd_1 < 0)
      {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 3;
      }

      int fd_2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd_2 < 0)
      {
        fprintf(stderr, "Failed to open %s for writing\n", argv[2]);
        return 4;
      }

      copy_all(fd_1, fd_2);

      if(close(fd_1) < 0)
      {
        fprintf(stderr, "Failure while closing %s\n", argv[1]);
        return 5;
      }

      if(close(fd_2) < 0)
      {
        fprintf(stderr, "Failure while closing %s\n", argv[2]);
        return 6;
      }
    break;
    }
    case S_IFIFO:
      if(mkfifo(argv[2], sb.st_mode) < 0)
      {
        fprintf(stderr, "Failed to create %s FIFO file\n", argv[2]);
        return 13;
      }
      break;

    case S_IFLNK:
      create_symlink(argv[1], argv[2]);
      break;
    case S_IFBLK:
    case S_IFCHR:
      if(mknod(argv[2], sb.st_mode, sb.st_rdev) < 0)
      {
        fprintf(stderr, "Failed to create new character device");
        return 12;
      }
      break;
    case S_IFDIR:
      printf("directory\n");
      return 14;
      break;
    case S_IFSOCK:
      printf("socket\n");
      return 15;
      break;
    default:
      fprintf(stderr, "File %s is not type of 'Block device', 'Character device', 'Regular', 'FIFO' or 'Symlink'\n", argv[1]); 
      return 16;
  }
  return 0;
}