#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  fprintf(stderr, "Тест 2\n");
  execlp("grep", "grep", argc > 1 ? argv[1] : "model name", "/proc/cpuinfo", NULL);
  perror("failed to exec grep");
  return 0;
}