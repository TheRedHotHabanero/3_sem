#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

volatile int g_last_signal;

void sig_handler(int signum)
{
  g_last_signal = signum;
}

int main() {
  struct sigaction action = {
    .sa_flags = SA_RESTART,
    .sa_handler = sig_handler,
  };
  sigfillset(&action.sa_mask);

  for (int i = 0; i <= _NSIG; i++) {
    if (sigaction(i, &action, NULL) < 0) {
      fprintf(stderr, "...%d: %s\n", i, strerror(errno));
    }
  }

  while (1) {
    pause();
    printf("\tSignal %d came\n", g_last_signal);
  }

  return 0;
}
