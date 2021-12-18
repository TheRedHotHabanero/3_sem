#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define NUMBER_OF_SIGNALS 32

// The behavior of this variable cannot be optimized
volatile int g_last_signal;
volatile siginfo_t *g_from_who;

const int signls[NUMBER_OF_SIGNALS] = {
    SIGABRT,   SIGALRM, SIGBUS,  SIGCHLD, SIGCLD,    SIGCONT, SIGFPE,
    SIGHUP,    SIGILL,  SIGINT,  SIGIO,   SIGIOT,    SIGPIPE, SIGPOLL,
    SIGPROF,   SIGPWR,  SIGQUIT, SIGSEGV, SIGSTKFLT, SIGTSTP, SIGSYS,
    SIGTERM,   SIGTRAP, SIGTTIN, SIGTTOU, SIGURG,    SIGUSR1, SIGUSR2,
    SIGVTALRM, SIGXCPU, SIGXFSZ, SIGWINCH};

void sig_handler(int signum, siginfo_t *info, void *ucontext) {
  g_last_signal = signum;
  g_from_who = info;

  if (ucontext != NULL)
    ucontext = NULL;
}

int main() {
  int result = 0;

  struct sigaction recieved = {};

  recieved.sa_flags = SA_SIGINFO;
  recieved.sa_sigaction = sig_handler;

  for (int i = 0; i < NUMBER_OF_SIGNALS; i++) {
    if (sigaction(signls[i], &recieved, NULL) < 0) {
      perror("sigaction");
      return -1;
    }
  }

  while (1) {
    pause();
    printf("\tSignal %d came\n", g_last_signal);
  }

  return 0;
}
