#define _GNU_SOURCE
#define BUF_SIZE 65536
#define MEMBLOCK 4096

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

ssize_t write_all(int fd, const void *buf, size_t count)
{
  size_t bytes_written = 0;
  const uint8_t *buf_addr = (const uint8_t *)buf;
  while (bytes_written < count) {
    ssize_t res = write(fd, buf_addr + bytes_written, count - bytes_written);
    if (res < 0)
    {
      return res;
    }

    bytes_written += res;
  }
  return (ssize_t)bytes_written;
}

size_t copy_all(const int fd_1, const int fd_2, struct stat *sb)
{
  ssize_t copy_var = 1;
  while (copy_var > 0) {
    void *buf = calloc(MEMBLOCK, sizeof(char));
    assert(buf);
    copy_var = read(fd_1, buf, MEMBLOCK);
    if (copy_var < 0) {
      perror("Failed to read file");
      free(buf);
      return 7;
    }

    if (write_all(fd_2, buf, (size_t)copy_var) < 0)
    {
      perror("Failure to write");
      free(buf);
      return 8;
    }
    free(buf);
  }

  // copying rights to access
  if (fchmod(fd_2, sb->st_mode) < 0)
  {
    perror("Failure while copying access rights");
    return 9;
  }

  // copying time of appeal and modifications
  if (futimens(fd_2, (struct timespec[]){sb->st_atim, sb->st_mtim}) < 0)
  {
    perror("Failure while copying times");
    return 10;
  }

  // copying uid and gid
  if (fchown(fd_2, sb->st_uid, sb->st_gid) < 0)
  {
    perror("Failure to copy UID and GID");
    return 11;
  }
  return 0;
}

ssize_t crt_link(const char *pathname, const char *newname)
{
  // memory allocation
  char *buf = (char *)calloc(PATH_MAX, sizeof(char));
  ssize_t result = 0;

  // reading the link
  if (readlink(pathname, buf, PATH_MAX) < 0)
  {
    fprintf(stderr, "Failed to read link\n");
    result = 12;
  }

  // creating new link
  if (symlink(buf, newname) < 0)
  {
    fprintf(stderr, "Failed to create a  new link\n");
    result = 13;
  }

  free(buf);
  return result;
}

// Function which creates a new symlink
ssize_t crt_linkat(const int old_fd, const int new_fd, const char *pathname,
                   const char *newname)
{
  // memory allocation
  char *buf = (char *)calloc(PATH_MAX, sizeof(char));
  ssize_t result = 0;

  // reading the link
  if (readlinkat(old_fd, pathname, buf, PATH_MAX) < 0)
  {
    fprintf(stderr, "Failed to read link\n");
    result = 12;
  }

  // creating new link
  if (symlinkat(buf, new_fd, newname) < 0)
  {
    fprintf(stderr, "Failed to create a new link\n");
    result = 13;
  }

  free(buf);
  return result;
}

char dtype_char(unsigned dtype)
{
  switch (dtype)
  {
    case DT_BLK:    return 'b';
    case DT_CHR:    return 'c';
    case DT_DIR:    return 'd';
    case DT_FIFO:   return 'p';
    case DT_LNK:    return 'l';
    case DT_REG:    return '-';
    case DT_SOCK:   return 's';
  }
  return '?';
}

char mode_char(unsigned mode)
{
  switch (mode & S_IFMT)
  {
    case S_IFBLK:   return 'b';
    case S_IFCHR:   return 'c';
    case S_IFDIR:   return 'd';
    case S_IFIFO:   return 'p';
    case S_IFLNK:   return 'l';
    case S_IFREG:   return '-';
    case S_IFSOCK:  return 's';
  }
  return '?';
}

struct linux_dirent64
{
  ino64_t d_ino;           // 64-bit inode number
  off_t d_off;             // 64-bit offset to next structure
  unsigned short d_reclen; // Size of this dirent
  unsigned char d_type;    // File type
  char d_name[];           // Filename (null-terminated)
};

int main(int argc, char *argv[])
{
  const char *dir_name = ".";
  char buf[BUF_SIZE];

  if (argc == 2)
  {
    dir_name = argv[1];
  }

  int dir_fd = open(dir_name, O_RDONLY | O_DIRECTORY);

  if (!dir_fd)
  {
    perror("open");
    return 1;
  }

  struct linux_dirent64 *entry = NULL;
  long int nread = 0;

  int pos = 0;
  while (1)
  {
    nread = getdents64(dir_fd, buf, BUF_SIZE);

    if (nread == -1)
    {
      perror("getdents64");
      close(dir_fd);
      return 2;
    }

    if (nread == 0)
      break;

    while (pos < nread)
    {
      entry = (struct linux_dirent64 *)(buf + pos);

      if (entry->d_type == '?')
      {
        struct stat sb;
        if (fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) < 0)
        {
          perror("fstatat");
          printf("?");
        }
        else
          entry->d_type = (unsigned char)mode_char(sb.st_mode);
      }

      printf("%c|", dtype_char(entry->d_type));
      printf("%s\n", entry->d_name);
      pos += entry->d_reclen;
    }
  }

  close(dir_fd);
  return 0;
}