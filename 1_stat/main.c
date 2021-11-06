#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>      //lstat
#include <sys/sysmacros.h> //fstat
#include <sys/types.h>     //stat
#include <time.h>
#define PERMS_BITS 1023

int nano_time(char* out_buf, size_t buf_len, const struct timespec* file_time)
{
  struct tm* time_ = localtime(&(file_time->tv_sec));
  char current_time[sizeof("YYYY-MM-DD HH:MM:SS")];
  char add_time[sizeof("HHMM")];

  strftime(current_time, sizeof(current_time),"%F %T", time_);
  strftime(add_time, sizeof(add_time), "%z", time_);

  return snprintf(out_buf, buf_len, "%s.%09ld %s", current_time, file_time->tv_nsec, add_time);
}

void perms(char* buf, const unsigned mode)
{
  buf[0] = mode & S_IRUSR ? 'r' : '-';
  buf[1] = mode & S_IWUSR ? 'w' : '-';
  buf[2] = mode & S_IXUSR ? 'x' : '-';
  buf[3] = mode & S_IRGRP ? 'r' : '-';
  buf[4] = mode & S_IWGRP ? 'w' : '-';
  buf[5] = mode & S_IXGRP ? 'x' : '-';
  buf[6] = mode & S_IROTH ? 'r' : '-';
  buf[7] = mode & S_IWOTH ? 'w' : '-';
  buf[8] = mode & S_IXOTH ? 'x' : '-';
}

int main(int argc, char *argv[])
{
  struct stat sb;
  char string[sizeof("YYYY-MM-DD HH:MM:SS.nnnnnnnnn +HHMM")];
  char perm_str[sizeof("rwxrwxrwx")];

  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (lstat(argv[1], &sb) == -1)
  {
    perror("lstat");
    exit(EXIT_FAILURE);
  }

  printf("File name:                %s\n", argv[1]);
	printf("ID of containing device:  [%lx,%lx]\n",
        (long) major(sb.st_dev), (long) minor(sb.st_dev));
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

  printf("I-node number:            %ld\n", (long) sb.st_ino);
  printf("Mode:                     %lo (octal)\n",
        (unsigned long) sb.st_mode);
  printf("Link count:               %ld\n", (long) sb.st_nlink);
  perms(perm_str, sb.st_mode);
  printf("Access:                   %o/%s\n", sb.st_mode & PERMS_BITS, perm_str);
	printf("Ownership:                UID=%ld   GID=%ld\n",
        (long) sb.st_uid, (long) sb.st_gid);
	printf("Preferred I/O block size: %ld bytes\n",
        (long) sb.st_blksize);
  printf("File size:                %lld bytes\n",
        (long long) sb.st_size);
  printf("Blocks allocated:         %lld\n",
        (long long) sb.st_blocks);

	//------------------------------------------------------special time format
  nano_time(string, sizeof(string), &sb.st_ctim);
  printf("Last status change:       %s\n", string);
  nano_time(string, sizeof(string), &sb.st_atim);
  printf("Last file access:         %s\n", string);
  nano_time(string, sizeof(string), &sb.st_mtim);
  printf("Last file modification:   %s\n", string);
	//-------------------------------------------------------------------------

	exit(EXIT_SUCCESS);
}
