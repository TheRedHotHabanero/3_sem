#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#define NGROUPS 20
#define DEBUG

int main() {
  // UID
  uid_t real_user_id = getuid();
  uid_t effecrive_user_id = geteuid();
  printf("Real user ID:                   %u\n", real_user_id);
  printf("Effective user ID:              %u\n", effecrive_user_id);

  // PID, PPID
  pid_t process_id = getpid();
  pid_t parent_pid = getppid();
  printf("Process ID:                     %d\n", process_id);
  printf("Parent's process ID:            %d\n", parent_pid);

  // GID
  const char *grp_name;
  struct group *grp_info = getgrgid(getpgid(process_id));
  if (grp_info == NULL) {
    grp_name = "?";
  } else {
    grp_name = grp_info->gr_name;
  }
  printf("Group ID =                      %d, %s\n", getpgid(process_id), grp_name);

  // User name
  struct passwd *pass_info;
  const char *user_name;
  pass_info = getpwuid(getuid());
  if (pass_info == NULL) {
    user_name = "?";
  } else {
    user_name = pass_info->pw_name;
  }

  // All groups
  int j;
  int ngroups;
  gid_t *groups;
  struct passwd *pw;
  struct group *gr;
  ngroups = NGROUPS;
  groups = malloc(ngroups * sizeof(gid_t));
  assert(groups);

  // Contains ID primary user group
  pw = getpwnam(user_name);
  if (pw == NULL) {
    perror("getpwnam");
    return 0;
  }

  // List of group
  if (getgrouplist(user_name, pw->pw_gid, groups, &ngroups) == -1) {
    fprintf(stderr, "getgrouplist() returned -1\n");
    return 0;
  }

  // Printing list
  fprintf(stderr, "ngroups = %d\n", ngroups);
  for (j = 0; j < ngroups; j++) {
    printf("Group (name):                   %d", groups[j]);
    gr = getgrgid(groups[j]);
    if (gr != NULL) {
      printf(" (%s)", gr->gr_name);
    }
    printf("\n");
  }

  // Umask
  mode_t u_mask = umask(ALLPERMS);
  printf("Umask:                          %o\n", u_mask);
  u_mask = umask(u_mask);

  return 0;
}