#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 128

#ifndef DAEMON_NAME
#define DAEMON_NAME "ALPHA"
#endif

#ifndef SLEEP_TIME
#define SLEEP_TIME 2
#endif

int main(int ac, char **av, char **env) {
  pid_t pid = getpid();
  char msg_out[BUF_SIZE] = {0};
  char msg_err[BUF_SIZE] = {0};
  int len_out = sprintf(msg_out, "[%-6d] - %-15s - STDOUT - RUNNING\n", pid,
			DAEMON_NAME),
      len_err = sprintf(msg_err, "[%-6d] - %-15s - STDERR - RUNNING\n", pid,
			DAEMON_NAME);

  while (1) {
    write(STDOUT_FILENO, msg_out, len_out);
    write(STDERR_FILENO, msg_err, len_err);
    sleep(SLEEP_TIME);
  }
  return EXIT_SUCCESS;
}
