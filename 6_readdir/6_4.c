#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
  return DT_UNKNOWN;
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

size_t print_dir(DIR *dir_fd, unsigned level) {
  int fd = dirfd(dir_fd);
  struct dirent *entry;

  while ((entry = readdir(dir_fd)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char entry_type = dtype_char(entry->d_type);
    if (entry_type == DT_UNKNOWN) {
      struct stat sb;
      if (fstatat(fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) < 0) {
        perror("fstatat");
      } else
        entry_type = mode_char(sb.st_mode);
    }
    printf("%c* ", entry_type);
    if (level > 1)
      for (unsigned i = 0; i < level - 1; i++)
      printf("*  ");

    if (level > 0)
      printf("*");

    printf("%s\n", entry->d_name);
    if (entry_type == 'd') {
      int nest_dirfd = openat(fd, entry->d_name, O_RDONLY | O_DIRECTORY);
      DIR *dir_fd = fdopendir(fd);
    }
  }

  closedir(dir_fd);
  return 0;
}

int main(int argc, char *argv[])
{
  const char *dir_name = ".";

  if (argc == 2) {
    dir_name = argv[1];
  }

  print_dir((DIR*)dir_name, 0);
  return 0;
}