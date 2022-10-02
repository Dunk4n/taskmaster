#include "taskmaster.h"

static void init_thread(const struct program_specification *pgm,
                        struct thread_data *thrd) {
    for (uint32_t i = 0; i < pgm->number_of_process; i++) {
        thrd->rid = i;
        thrd->restart_counter = pgm->start_retries;
    }
}

/* If config says that process logs somewhere, init_log() open and store these
 * files */
static uint8_t init_log(struct program_specification *pgm) {
    pgm->log.out = UNINITIALIZED_FD;
    pgm->log.out = UNINITIALIZED_FD;

    if (pgm->str_stdout) {
        pgm->log.out = open(pgm->str_stdout, O_RDWR | O_CREAT | O_APPEND, 0755);
        if (pgm->log.out == FD_ERR) { /* TODO: do something */
            return EXIT_FAILURE;
        }
    }
    if (pgm->str_stderr) {
        pgm->log.err = open(pgm->str_stderr, O_RDWR | O_CREAT | O_APPEND, 0755);
        if (pgm->log.err == FD_ERR) { /* TODO: do something */
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static uint8_t init_thread_data(struct program_specification *pgm) {
    while (pgm) {
        pgm->thrd = (struct thread_data *)calloc(pgm->number_of_process,
                                                 sizeof(struct thread_data));
        if (!pgm->thrd) {
            /* TODO: free la liste, log et return failure */
            return EXIT_FAILURE;
        }
        if (init_log(pgm)) return EXIT_FAILURE;
        init_thread(pgm, pgm->thrd);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

/* launch the thread which controls and launch all others launch_threads */
uint8_t tm_server(struct taskmaster *tm) {
    if (init_thread_data(tm->programs.program_linked_list)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
