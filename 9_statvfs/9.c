#include <stdio.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  const char *fs_name = ".";
  char *file_name;

  struct statvfs sbfs;
  struct stat sb;

  if (argc > 1)
    fs_name = argv[1];

  if (argc == 2) {
    file_name = argv[1];
  } else if (argc == 3) {
    fs_name = argv[1];
    file_name = argv[2];
  }

  if (lstat(fs_name, &sb) < 0) {
    perror("lstat");
    return 0;
  }

  // Check the file system name
  if ((sb.st_mode & S_IFMT) != S_IFDIR) {
    fprintf(stderr, "[err] %s isn't a file system\n", fs_name);
    return -1;
  }

  if (statvfs(fs_name, &sbfs) < 0) {
    perror("statvfs");
    return -1;
  }

  printf("Filesystem block size:                          %lu\n", sbfs.f_bsize);
  printf("Fragment size:                                  %lu\n", sbfs.f_frsize);
  printf("Size of filesystem in fragment size units:      %lu\n", sbfs.f_blocks);
  printf("Number of free blocks:                          %lu\n", sbfs.f_bfree);
  printf("Number of free blocks for unprivileged users:   %lu\n", sbfs.f_bavail);
  printf("Number of inodes:                               %lu\n", sbfs.f_files);
  printf("Number of free inodes:                          %lu\n", sbfs.f_ffree);
  printf("Number of free inodes for unprivileged users:   %lu\n", sbfs.f_favail);
  printf("Filesystem ID:                                  %lu\n", sbfs.f_fsid);
  printf("Mount flags:                                    %lu\n", sbfs.f_flag);
  printf("Maximum filename length:                        %lu\n", sbfs.f_namemax);

  if (argc > 2) {
    const char *file_name = argv[2];
    struct stat sb;

    if (lstat(file_name, &sb) < 0) {
      perror("lstat");
      return -1;
    }

    char result = 1;

    if (sbfs.f_favail < 1)
      result = 0;

    if ((long int)(sbfs.f_bavail * sbfs.f_bsize) < sb.st_size)
      result = 0;

    printf("\nDoes the file fit:                              %s\n",
           result ? "Yes" : "No");
  }

  return 0;
}