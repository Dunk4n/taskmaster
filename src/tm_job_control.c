#include "taskmaster.h"

/* launch the thread which controls and launch all others launch_threads */
uint8_t tm_job_control(struct taskmaster *tm) {
    (void)tm;
    return EXIT_SUCCESS;
}
