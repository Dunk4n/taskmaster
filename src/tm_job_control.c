#include "taskmaster.h"

static void *routine_launch_thrd(void *arg) {
    /* here code to fork pipe dup2 execve... */
    (void)arg;
    return NULL;
}

/* create launcher_threads */
static uint8_t start_launcher_thread(struct program_specification *pgm,
                                     const pthread_attr_t *attr) {
    for (uint32_t i = 0; i < pgm->number_of_process; i++) {
        if (pthread_create(&pgm->thrd[i].tid, attr, routine_launch_thrd, pgm))
            log_error("failed to create launcher thread", __FILE__, __func__,
                      __LINE__);
    }
    return EXIT_SUCCESS;
}

/* wrapper around start_launcher_thread() to loop thru the pgm_spec linked list
 */
static uint8_t launch_all(const struct program_list *pm) {
    struct program_specification *pgm = pm->program_linked_list;

    for (uint32_t i = 0; i < pm->number_of_program; i++) {
        start_launcher_thread(pgm, &pm->attr);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

static void *routine_master_thrd(void *arg) {
    launch_all(arg);
    /* Here, code to monitor all that shit */
    return NULL;
}

/* launch the thread which controls and launch all others launch_threads */
uint8_t tm_job_control(struct program_list *pm) {
    if (pthread_create(&pm->master_thread, &pm->attr, routine_master_thrd, pm))
        log_error("failed to create master thread", __FILE__, __func__,
                  __LINE__);
    return EXIT_SUCCESS;
}

/* role du master thread communiquer avec le client via la structure runtime */
/* (le client set des variables dedans, que le master thread va */
/* checker continuellement). Ex voir si il y a un programme a kill. */

/* Le master thread est detached et lance tout les launch_thread en detached. */
/* Si le master thread est kill il y a 2 façons de gérer: soit il kill aussi */
/* son parent (et maybe tout les process), soit il créé un autre master thread.
 */
/* Après, il y a un signal qui n'est pas gérable donc maybe il faut que le
 * master */
/* thread envoie régulièrement au parent un signe de vie ? */
