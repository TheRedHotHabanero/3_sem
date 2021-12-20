#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>
//#define BITS 1023 // 2_ or 8_ perms sys/stat.h
// gcc -Werror -Wall -Wextra -Wnarrowing -Wconversion -Wwrite-strings -Wcast-qual -Wundef -Wstrict-prototypes -Wbad-function-cast -Wlogical-op -Wreturn-type -g -fsanitize=address,undefined main.c -o a.out
// gcc -Werror -Wall -Wextra -Wnarrowing -Wconversion -Wwrite-strings -Wcast-qual -Wundef -Wstrict-prototypes -Wbad-function-cast -Wlogical-op -Wreturn-type -g main.c -o main.exe && valgrind ./main.exe main.c
// clang -Werror -Wall -Wextra -Wnarrowing -Wconversion -Wwrite-strings -Wcast-qual -Wundef -Wstrict-prototypes -Wbad-function-cast -Wreturn-type -g -fsanitize=memory main.c -o main.exe
// https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/stat.h.html

#ifndef S_BLKSIZE
#include <sys/param.h>
#ifdef DEV_BSIZE
#define S_BLKSIZE DEV_BSIZE
#else
#error "Couldn't determine block size for this platfrom"
#endif
#endif

int nano_time(char* out_buf, size_t buf_len, const struct timespec* file_time)
{
  struct tm* time_ = localtime(&(file_time->tv_sec));
  char current_time[sizeof("YYYY-MM-DD HH:MM:SS")];
  char add_time[sizeof("HHMM")];

  strftime(current_time, sizeof(current_time),"%F %T", time_);
  strftime(add_time, sizeof(add_time), "%z", time_);

  return snprintf(out_buf, buf_len, "%s.%09ld %s", current_time, file_time->tv_nsec, add_time);
}

void bufs(char* buf, const mode_t mode)
{
  *buf++ = mode & S_IRUSR ? 'r' : '-';
  *buf++ = mode & S_IWUSR ? 'w' : '-';
  *buf++ = mode & S_IXUSR ? 'x' : '-';
  *buf++ = mode & S_IRGRP ? 'r' : '-';
  *buf++ = mode & S_IWGRP ? 'w' : '-';
  *buf++ = mode & S_IXGRP ? 'x' : '-';
  *buf++ = mode & S_IROTH ? 'r' : '-';
  *buf++ = mode & S_IWOTH ? 'w' : '-';
  *buf++ = mode & S_IXOTH ? 'x' : '-';
  *buf = '\0';
  // терм 0
  // заменить на цикл из трех строчек + 3 бита сетюид, сетгид и чет еще забыла
  // cppcheck
}

int main(int argc, char *argv[])
{
  struct stat sb = {};
  char string[sizeof("YYYY-MM-DD HH:MM:SS.nnnnnnnnn +HHMM")];
  char perm_str[sizeof("rwxrwxrwx")];

  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
    return(1);
  }

  if (lstat(argv[1], &sb) == -1)
  {
    perror("lstat");
    return(2);
  }

  printf("File name:                %s\n", argv[1]);
  printf("ID of containing device:  [%xh,%xh]\n", major(sb.st_dev), minor(sb.st_dev));
  printf("File type:                ");

  switch (sb.st_mode & S_IFMT)
  {
		case S_IFBLK:  printf("block device\n");            break;
    case S_IFCHR:  printf("character device\n");        break;
    case S_IFDIR:  printf("directory\n");               break;
    case S_IFIFO:  printf("FIFO/pipe\n");               break;
    case S_IFLNK:  printf("symlink\n");                 break;
    case S_IFREG:  printf("regular file\n");            break;
    case S_IFSOCK: printf("socket\n");                  break;
    default:       printf("unknown?\n");                break;
  }

  printf("I-node number:            %ju\n", (long) sb.st_ino);
  printf("Mode:                     %o (octal)\n", sb.st_mode);
  printf("Link count:               %ju\n", (uintmax_t) sb.st_nlink);
  bufs(perm_str, sb.st_mode);
  printf("Access:                   %04o/%s\n", sb.st_mode & ALLPERMS, perm_str);
  printf("Ownership:                UID=%ju   GID=%ju\n", (uintmax_t) sb.st_uid, (uintmax_t) sb.st_gid);
  printf("Preferred I/O block size: %ju bytes\n", (uintmax_t) sb.st_blksize);
  printf("File size:                %ju bytes\n", (uintmax_t) sb.st_size);
  printf("Blocks allocated:         %ju bytes)\n", (uintmax_t) sb.st_blocks);

	//------------------------------------------------------special time format
  nano_time(string, sizeof(string), &sb.st_ctim);
  printf("Last status change:       %s\n", string);
  nano_time(string, sizeof(string), &sb.st_atim);
  printf("Last file access:         %s\n", string);
  nano_time(string, sizeof(string), &sb.st_mtim);
  printf("Last file modification:   %s\n", string);
	//-------------------------------------------------------------------------

  return(0);
}
