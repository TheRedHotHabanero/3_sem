#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIR_MODE 0777
#define MEMBLOCK 4096

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

ssize_t write_all(int fd, const void *buf, size_t count) {
  size_t bytes_written = 0;
  const uint8_t *buf_addr = (const uint8_t *)buf;
  while (bytes_written < count) {
    ssize_t res = write(fd, buf_addr + bytes_written, count - bytes_written);
    if (res < 0) {
      return res;
    }

    bytes_written += res;
  }
  return (ssize_t)bytes_written;
}

size_t copy_all(const int fd_1, const int fd_2, struct stat *sb) {
  ssize_t copy_var;
  void* buf = calloc(MEMBLOCK, sizeof(char));
  assert(buf);
  int res = 0;
  while (copy_var = read(fd_1, buf, MEMBLOCK))
  {
    if (write_all(fd_2, buf, (size_t)copy_var) < 0)
    {
      perror("Failure to write");
      free(buf);
      return 0;
    }
  }
  if (copy_var < 0)
  {
    perror("Failure to read file");
    res = 7;
  }
  free(buf);

  // Copying rights to access
  if (fchmod(fd_2, sb->st_mode) < 0) {
    perror("Failure while copying access rights");
    return 9;
  }

  // Copying time of appeal and modifications
  if (futimens(fd_2, (struct timespec[]){sb->st_atim, sb->st_mtim}) < 0) {
    perror("Failure while copying times");
    return 10;
  }

  // Copying uid and gid
  if (fchown(fd_2, sb->st_uid, sb->st_gid) < 0) {
    perror("Failure to copy UID and GID");
    return 11;
  }
  return 0;
}

// Function which creates a new symlink
ssize_t crt_linkat(const int old_fd, const int new_fd, const char *pathname,
                   const char *newname) {
  // Memory allocation
  char *buf = (char *)calloc(PATH_MAX, sizeof(char));
  ssize_t result = 0;

  // Reading the link
  if (readlinkat(old_fd, pathname, buf, PATH_MAX) < 0) {
    fprintf(stderr, "Failed to read link\n");
    result = 12;
  }

  // Creating new link
  if (symlinkat(buf, new_fd, newname) < 0) {
    fprintf(stderr, "Failed to create a new link\n");
    result = 13;
  }

  free(buf);
  return result;
}

ssize_t copy_dir(DIR *old_dir, DIR *new_dir) {
  ssize_t result = 0;

  struct dirent *entry;
  struct stat sb;

  // Obtaining file descriptors of our old and new directories
  int old_dir_fd = dirfd(old_dir);
  int new_dir_fd = dirfd(new_dir);

  while ((entry = readdir(old_dir)) != NULL) {
    char entry_type = '?';
    if (fstatat(old_dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) < 0) {
      perror("fstatat");
    } else
      entry_type = mode_char(sb.st_mode);

    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {

      switch (entry_type) {
      case '-': {
        int old_file_fd = openat(old_dir_fd, entry->d_name, O_RDONLY);
        int new_file_fd = openat(new_dir_fd, entry->d_name,
                                 O_WRONLY | O_CREAT | O_TRUNC, 0777);

        if (new_file_fd < 0) {
          perror("openat");
          result = -1;
        }

        if (copy_all(old_file_fd, new_file_fd, &sb) < 0) {
          fprintf(stderr, "Failure while copying file <%s>\n", entry->d_name);
          result = -1;
        }

        close(old_file_fd);
        close(new_file_fd);
      } break;

      // Symlinks
      case 'l': {
        if (crt_linkat(old_dir_fd, new_dir_fd, entry->d_name, entry->d_name) <
                0 &&
            errno != EEXIST) {
          fprintf(stderr, "Failure while copying link <%s>\n", entry->d_name);
          result = -1;
        }

      } break;

      // Block devices and character devices can be created by the same function
      // so they merged
      case 'b':
      case 'c': {
        if (mknodat(new_dir_fd, entry->d_name, sb.st_mode, sb.st_rdev) < 0 &&
            errno != EEXIST) {
          perror("mknodat");
          fprintf(stderr, "Failed to copy block/character device <%s>\n",
                  entry->d_name);
          result = -1;
        }

      } break;

      // fifo/pipe
      case 'p': {
        if (mkfifoat(new_dir_fd, entry->d_name, sb.st_mode) < 0 &&
            errno != EEXIST) {
          perror("mkfifoat");
          result = -1;
        }

      } break;

      // directiories
      case 'd': {
        int old_nest_dirfd =
            openat(old_dir_fd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (old_nest_dirfd < 0) {
          perror("openat");
          result = -1;
        }
        DIR *old_nest_dir = fdopendir(old_nest_dirfd);

        // If directory already exists, then it will
        // be just opened. Else it will be created by mkdirat()
        if (mkdirat(new_dir_fd, entry->d_name, DIR_MODE) < 0 &&
            errno != EEXIST) {
          perror("mkdirat");
          result = -1;
        }

        int new_nest_dirfd =
            openat(new_dir_fd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (new_nest_dirfd < 0) {
          perror("openat");
          result = -1;
        }
        DIR *new_nest_dir = fdopendir(new_nest_dirfd);

        if (copy_dir(old_nest_dir, new_nest_dir) < 0) {
          fprintf(stderr, "Failure while copying directory <%s>\n",
                  entry->d_name);
          result = -1;
        }

        if (closedir(old_nest_dir) < 0) {
          perror("closedir");
          result = -1;
        }

        if (closedir(new_nest_dir) < 0) {
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

int main(int argc, char *argv[]) {
  const char *old_dir_name = ".";
  const char *new_dir_name = NULL;

  // If no parameters added
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <old dir pathname> <new dir pathname>\n",
            argv[0]);
    return 1;
  }

  if (argc == 3) {
    old_dir_name = argv[1];
    new_dir_name = argv[2];
  }

  // Obtaining DIR* pointer of directory program will copy from and opening it
  DIR *old_dir_fd = opendir(old_dir_name);

  if (mkdir(new_dir_name, DIR_MODE) < 0 && errno != EEXIST) {
    perror("mkdir");
    return -1;
  }

  DIR *new_dir_fd = opendir(new_dir_name);

  if (copy_dir(old_dir_fd, new_dir_fd) < 0) {
    fprintf(stderr, "\n\nFailure while copying directory or directory was "
                    "copied partialy\n\n");
  }

  closedir(old_dir_fd);
  closedir(new_dir_fd);

  return 0;
}