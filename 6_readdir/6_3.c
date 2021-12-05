#define _GNU_SOURCE
#define BUF_SIZE 65536

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DIR_MODE 0777
#define MEMBLOCK 4096
#define NUMBER_OF_SIGNALS 32

const int __signals__[NUMBER_OF_SIGNALS] =
{
  SIGABRT,   SIGALRM, SIGBUS,  SIGCHLD, SIGCLD,    SIGCONT, SIGFPE,
  SIGHUP,    SIGILL,  SIGINT,  SIGIO,   SIGIOT,    SIGPIPE, SIGPOLL,
  SIGPROF,   SIGPWR,  SIGQUIT, SIGSEGV, SIGSTKFLT, SIGTSTP, SIGSYS,
  SIGTERM,   SIGTRAP, SIGTTIN, SIGTTOU, SIGURG,    SIGUSR1, SIGUSR2,
  SIGVTALRM, SIGXCPU, SIGXFSZ, SIGWINCH
};

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

ssize_t copy_dir(DIR *old_dir, DIR *new_dir)
{
  ssize_t result = 0;

  struct dirent *entry;
  struct stat sb;

  // obtaining file descriptors of our old and new directories
  int old_dir_fd = dirfd(old_dir);
  int new_dir_fd = dirfd(new_dir);

  while ((entry = readdir(old_dir)) != NULL)
  {
    char entry_type = '?';
    if (fstatat(old_dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) < 0)
    {
      perror("fstatat");
    }
    else
    {
      entry_type = mode_char(sb.st_mode);
    }

    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
    {
      switch (entry_type)
      {
      // regular files
      case '-': {
        int old_file_fd = openat(old_dir_fd, entry->d_name, O_RDONLY);
        int new_file_fd = openat(new_dir_fd, entry->d_name,
                                 O_WRONLY | O_CREAT | O_TRUNC, 0777);

        if (new_file_fd < 0)
        {
          perror("openat");
          result = -1;
        }

        if (copy_all(old_file_fd, new_file_fd, &sb) < 0)
        {
          fprintf(stderr, "Failure while copying file <%s>\n", entry->d_name);
          result = -1;
        }

        close(old_file_fd);
        close(new_file_fd);
        break;
      }

      // symlinks
      case 'l':
      {
        if (crt_linkat(old_dir_fd, new_dir_fd, entry->d_name, entry->d_name) <
                0 &&
            errno != EEXIST)
        {
          fprintf(stderr, "Failure while copying link <%s>\n", entry->d_name);
          result = -1;
        }
        break;
      }

      // block devices and character devices can be created by the same function
      // so they merged
      case 'b':
      case 'c':
      {
        if (mknodat(new_dir_fd, entry->d_name, sb.st_mode, sb.st_rdev) < 0 &&
            errno != EEXIST)
        {
          perror("mknodat");
          fprintf(stderr, "Failed to copy block/character device <%s>\n", entry->d_name);
          result = -1;
        }

      } break;

      // fifo/pipe
      case 'p':
      {
        if (mkfifoat(new_dir_fd, entry->d_name, sb.st_mode) < 0 &&
            errno != EEXIST)
        {
          perror("mkfifoat");
          result = -1;
        }

      } break;

      // directiories
      case 'd':
      {
        int old_nest_dirfd =
            openat(old_dir_fd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (old_nest_dirfd < 0)
        {
          perror("openat");
          result = -1;
        }
        DIR *old_nest_dir = fdopendir(old_nest_dirfd);

        // if directory with name <entry->d_name> already exists, then it will
        // be just opened. Otherwise, it will be created by mkdirat()
        if (mkdirat(new_dir_fd, entry->d_name, DIR_MODE) < 0 && errno != EEXIST)
        {
          perror("mkdirat");
          result = -1;
        }

        int new_nest_dirfd =
            openat(new_dir_fd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (new_nest_dirfd < 0)
        {
          perror("openat");
          result = -1;
        }
        DIR *new_nest_dir = fdopendir(new_nest_dirfd);

        if (copy_dir(old_nest_dir, new_nest_dir) < 0)
        {
          fprintf(stderr, "Failure while copying directory <%s>\n",
                  entry->d_name);
          result = -1;
        }

        if (closedir(old_nest_dir) < 0)
        {
          perror("closedir");
          result = -1;
        }

        if (closedir(new_nest_dir) < 0)
        {
          perror("closedir");
          result = -1;
        }
      } break;

      default:
        printf("Unknown file type\n");
      }
    }
  }

  return result;
}

struct linux_dirent64
{
  ino64_t d_ino;           /* 64-bit inode number */
  off_t d_off;             /* 64-bit offset to next structure */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char d_type;    /* File type */
  char d_name[];           /* Filename (null-terminated) */
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