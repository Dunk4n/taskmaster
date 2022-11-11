#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

volatile int stop = 0;

void handler(int sig)
    {
    (void) sig;
    stop = 1;
    printf("CATCH SIGNAL\n");
    sleep(3);
    printf("END CATCH SIGNAL\n");
    }

int main(void)
    {
    if(signal(SIGUSR1, handler) == SIG_ERR)
        {
        printf("SIGNAL FAILED\n");
        return 1;
        }

    int i = 0;
    while(i < 200 && stop == 0)
        {
        sleep(1);
        printf("Waiting [%d] ...\n", i);
        fflush(stdout);
        i++;
        }
    return 1;
    }
