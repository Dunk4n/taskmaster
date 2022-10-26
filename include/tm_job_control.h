#include <inttypes.h>
#include <pthread.h>
#include <string.h>

#ifndef TM_JOB_CONTROL_H
#define TM_JOB_CONTROL_H

/* usleep() value for master_thread listening loop - in us */
#define CLIENT_LISTENING_RATE (200)
#define START_SUPERVISOR_RATE (400)
#define STOP_SUPERVISOR_RATE (400)
#define EXIT_MASTER_RATE (200)

#ifdef DEVELOPEMENT
#define debug_thrd()                                                         \
    do {                                                                     \
        printf("[%-14s- %-2d] - tid %lu - pid %d - cnt %d\n", pgm->str_name, \
               pgm->thrd[id].rid, pgm->thrd[id].tid, pgm->thrd[id].pid,      \
               pgm->thrd[id].restart_counter);                               \
        fflush(stdout);                                                      \
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
#define PGM_SPEC_SET(name, value)                               \
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
#define THRD_DATA_SET(name, value)  \
    do {                            \
        pgm->thrd[id].name = value; \
    } while (0)

/* thread_data getter. name is the name of the struct variable name */
#define THRD_DATA_GET(name) pgm->thrd[id].name

/*
 * update program_state data into struct program_specification.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define PGM_STATE_SET(name, value)                 \
    do {                                           \
        pthread_mutex_lock(&pgm->mtx_pgm_state);   \
        pgm->program_state.name = value;           \
        pthread_mutex_unlock(&pgm->mtx_pgm_state); \
    } while (0)

/*
 * program_state getter (thru a program_specification struct).
 * name is the name of the struct variable name
 **/
#define PGM_STATE_GET(name) pgm->program_state.name
/* #define PGM_STATE_GET(name) pgm_state_getter(pgm, name) */

/*
** LOG MACROS
*/

#define BUF_LOG_LEN 256
#define TM_LOG(func, fmt, ...)                                       \
    do {                                                             \
        pthread_mutex_lock(&node->mtx_log);                          \
        char buf[BUF_LOG_LEN] = {0};                                 \
        time_t t = time(NULL);                                       \
        char *curr_time = asctime(localtime(&t));                    \
        uint32_t len = strlen(curr_time) - 5;                        \
                                                                     \
        strncpy(buf, curr_time, len);                                \
        snprintf(buf + len, BUF_LOG_LEN, "- [" func "] - " fmt "\n", \
                 __VA_ARGS__);                                       \
        write(node->tm_fd_log, buf, strlen(buf));                    \
        pthread_mutex_unlock(&node->mtx_log);                        \
    } while (0)

#define TM_THRD_LOG(status)                                                \
    TM_LOG("launcher thread",                                              \
           "[%s pid[%d]] - tid[%lu] - restart_counter[%d] • [" status "]", \
           PGM_SPEC_GET(str_name), THRD_DATA_GET(pid), THRD_DATA_GET(tid), \
           THRD_DATA_GET(restart_counter));

#define TM_CHILDCONTROL_LOG(status)                                           \
    TM_LOG("child supervisor",                                                \
           "[%s pid[%d]] - tid[%lu] - restart_counter[%d] • [" status " %d]", \
           PGM_SPEC_GET(str_name), THRD_DATA_GET(pid), THRD_DATA_GET(tid),    \
           THRD_DATA_GET(restart_counter), child_ret);

#define TM_STOP_LOG(status)                                              \
    TM_LOG("stop supervisor", "[%s] - stop_time[%d sec] • [" status "]", \
           PGM_SPEC_GET(str_name), PGM_SPEC_GET(stop_time));

#define TM_START_LOG(status)                                                 \
    TM_LOG(                                                                  \
        "start supervisor",                                                  \
        "[%s pid[%d]] - tid[%lu] - rank[%d] - start_time[%d sec] • [" status \
        "]",                                                                 \
        PGM_SPEC_GET(str_name), time_control->pid, THRD_DATA_GET(tid),      \
        THRD_DATA_GET(rid), PGM_SPEC_GET(start_time));

typedef enum client_event : uint8_t {
    CLIENT_NOTHING = 0,
    CLIENT_START,
    CLIENT_RESTART,
    CLIENT_STOP,
    CLIENT_MAX_EVENT
} e_client_event;

#endif
