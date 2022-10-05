#ifndef TM_JOB_CONTROL_H
#define TM_JOB_CONTROL_H

/* usleep() value for master_thread listening loop - in ms */
#define CLIENT_LISTENING_RATE (100)

#ifdef DEVELOPEMENT
#define debug_thrd()                                                     \
    do {                                                                 \
        printf("[%-14s- %-2d] - tid %lu - pid %d\n", pgm->str_name,      \
               pgm->thrd[id].rid, pgm->thrd[id].tid, pgm->thrd[id].pid); \
    } while (0)
#else
#define debug_thrd()
#endif

#define exit_thrd(pgm, rid, msg, file, func, line) \
    do {                                           \
        exit_thread(pgm, rid);                     \
        err_display(msg, file, func, line);        \
        return (NULL);                             \
    } while (0)

/*
 * update data in struct program_specification.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define PGM_SPEC_UPDATE(name, value)                            \
    do {                                                        \
        pthread_mutex_lock(&node->mutex_program_linked_list);   \
        pgm->name = value;                                      \
        pthread_mutex_unlock(&node->mutex_program_linked_list); \
    } while (0)

/*
 * program_specification getter (thru a program_specification struct).
 * name is the name of the struct variable name
 **/
#define PGM_SPEC_GET(name) pgm->name

/*
 * update data in struct thread_data.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define THRD_DATA_UPDATE(name, value) \
    do {                              \
        pgm->thrd[id].name = value;   \
    } while (0)

/* thread_data getter. name is the name of the struct variable name */
#define THRD_DATA_GET(name) pgm->thrd[id].name

/*
 * update program_state data into struct program_specification.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define PGM_STATE_UPDATE(name, value)                           \
    do {                                                        \
        pthread_mutex_lock(&node->mutex_program_linked_list);   \
        pgm->program_state.name = value;                        \
        pthread_mutex_unlock(&node->mutex_program_linked_list); \
    } while (0)

/*
 * program_state getter (thru a program_specification struct).
 * name is the name of the struct variable name
 **/
#define PGM_STATE_GET(name) pgm->program_state.name

typedef enum client_event {
    CLIENT_NOTHING = 0,
    CLIENT_START,
    CLIENT_RESTART,
    CLIENT_STOP,
    CLIENT_MAX_EVENT
} e_client_event;

#endif
