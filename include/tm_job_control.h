#ifndef TM_JOB_CONTROL_H
#define TM_JOB_CONTROL_H

#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

/* usleep() value for master_thread listening loop - in us */
#define CLIENT_LISTENING_RATE (2000)
#define START_SUPERVISOR_RATE (4000)
#define STOP_SUPERVISOR_RATE (4000)
#define EXIT_MASTER_RATE (2000)
#define KILL_TIME_LIMIT (1000000)

typedef enum client_event {
    CLIENT_NOTHING = 0,
    CLIENT_START,
    CLIENT_RESTART,
    CLIENT_STOP,
    CLIENT_MAX_EVENT
} e_client_event;

enum pgm_states {
    started = 1 << 0,
    need_to_restart = 1 << 1,
    restarting = 1 << 2,
    need_to_stop = 1 << 3,
    stopping = 1 << 4,
    need_to_start = 1 << 5,
    starting = 1 << 6,
    need_to_be_removed = 1 << 7
};

typedef struct timeval tm_timeval_t;

/* runtime data relative to a thread. One launcher thread has one timer */
struct thread_data {
    /* pthread_mutex_t rw_thrd; */
    pthread_rwlock_t rw_thrd;
    /* constant synchronization between timer & launcher */
    pthread_barrier_t sync_barrier;

    struct program_list *node;         /* pointer to the node */
    struct program_specification *pgm; /* pointer to the related pgm data */

    uint32_t rid;  /* rank id of current thread/proc. Index for an array */
    pthread_t tid; /* thread id of current thread */
    uint32_t pid;  /* pid of current process */
    int32_t restart_counter; /* how many time the process can be restarted */
    tm_timeval_t start_timestamp; /* time when process started */

    pthread_mutex_t mtx_wakeup;
    pthread_cond_t
        cond_wakeup; /* conditon variable to signal thread to start */

    /*
     * This variable must be set with its macros
     * bits are ordered as following: eeeessss
     * states: stopped - started - stopping - starting.
     * events: restarting.
     */
    atomic_uchar info;

    /*    timer    */

    /* semaphore to synchronize timer & launcher thread at init */
    sem_t sync;
    pthread_mutex_t mtx_timer;
    pthread_cond_t cond_timer; /* conditon variable to unlock timer */
    pthread_t timer_id;        /* thread id of start_timer thread */
    bool proc_restart;         /* restart timer */
};

/* PROCESSUS STATES */

#define PROC_ST_STOPPED (0x00) /* 0000 */
#define PROC_ST_STARTED (0x01) /* 0001 */
/* When stopping, proc is also started - 0011 */
#define PROC_ST_STOPPING (0x03)
/* When starting, proc is also stopped - 0100 */
#define PROC_ST_STARTING (0x04)

/* THREAD EVENTS */

#define THRD_EV_NOEVENT (0x0) /* default. */
#define THRD_EV_STOP (0x1)    /* thread gets idle */
#define THRD_EV_RESTART (0x2) /* thread gets active */
#define THRD_EV_EXIT (0x3)    /* thread gets exited */

/* MASKS */

/* proc is active if is either started, starting or stopping - 0111 */
#define PROC_ACTIVE (0x07)
#define IS_PROC_ACTIVE(ptr) (((ptr)->info & PROC_ACTIVE) > 0)

/* INFO SETTERS & GETTERS */

/*
 * if proc state value to set is starting, it overides restarting event to 0,
 * otherwise not
 **/
#define SET_PROC_STATE(value)                                          \
    do {                                                               \
        thrd->info = (((thrd->info & 0xf0) + ((value)&0x0f)) *         \
                      ((value) != PROC_ST_STARTING)) +                 \
                     (((value)&0x0f) * ((value) == PROC_ST_STARTING)); \
    } while (0)
#define GET_PROC_STATE (thrd->info & 0x0f)

/* set event to info without overriding states */
#define SET_THRD_EVENT(value)                              \
    do {                                                   \
        thrd->info = ((value) << 4) + (thrd->info & 0x0f); \
    } while (0)

#define GET_THRD_EVENT (thrd->info >> 4)

/* EXIT & DEBUG MACROS */

#ifdef DEVELOPEMENT
#define debug_thrd()                                                        \
    do {                                                                    \
        printf("[%-14s- %-2d] - tid %lu - pid %d - cnt %d\n",               \
               PGM_SPEC_GET_T(str_name), THRD_DATA_GET(uint32_t, rid),      \
               THRD_DATA_GET(pthread_t, tid), THRD_DATA_GET(uint32_t, pid), \
               THRD_DATA_GET(int32_t, restart_counter));                    \
        fflush(stdout);                                                     \
    } while (0)
#else
#define debug_thrd()
#endif

#define exit_thrd(thrd, msg, file, func, line) \
    do {                                       \
        exit_thread(thrd);                     \
        err_display(msg, file, func, line);    \
        return (NULL);                         \
    } while (0)

/*
 * update data in struct thread_data.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define THRD_DATA_SET(name, value)              \
    do {                                        \
        pthread_rwlock_wrlock(&thrd->rw_thrd); \
        thrd->name = value;                     \
        pthread_rwlock_unlock(&thrd->rw_thrd); \
    } while (0)

/* generic getters functions to have lock-free value from struct thread_data */
#define THRD_DATA_GET_FUNC(type) thrd_data_get_##type
#define THRD_DATA_GET_CALL(type, value) THRD_DATA_GET_FUNC(type)(thrd, value)

#define THRD_DATA_GET_DECL(type)                                    \
    static type THRD_DATA_GET_FUNC(type)(struct thread_data * thrd, \
                                         type * value)
#define THRD_DATA_GET_IMPLEMENTATION            \
    THRD_DATA_GET_DECL(THRD_TYPE) {             \
        THRD_TYPE save;                         \
                                                \
        pthread_rwlock_rdlock(&thrd->rw_thrd); \
        save = *value;                          \
        pthread_rwlock_unlock(&thrd->rw_thrd); \
        return save;                            \
    }

/* thread_data getter. name is the name of the struct variable name */
#define THRD_DATA_GET(type, name) THRD_DATA_GET_CALL(type, &thrd->name)

/*
 * update data in struct program_specification.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define PGM_SPEC_SET_T(name, value)                                   \
    do {                                                              \
        pthread_mutex_lock(&thrd->node->mutex_program_linked_list);   \
        thrd->pgm->name = value;                                      \
        pthread_mutex_unlock(&thrd->node->mutex_program_linked_list); \
    } while (0)
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
#define PGM_SPEC_GET_T(name) thrd->pgm->name /* get from launcher thread */
#define PGM_SPEC_GET(name) pgm->name         /* get from master thread */

/*
 * update program_state data into struct program_specification.
 * @args:
 *   name    is the name of the variable from the struct that we want to update
 *   value   is the value we want to give to this variable
 **/
#define PGM_STATE_SET_T(name, value)                     \
    do {                                                 \
        pthread_mutex_lock(&thrd->pgm->mtx_pgm_state);   \
        thrd->pgm->program_state.name = value;           \
        pthread_mutex_unlock(&thrd->pgm->mtx_pgm_state); \
    } while (0)
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
/* #define PGM_STATE_GET_T(name) thrd->pgm->program_state.name */
/* #define PGM_STATE_GET(name) pgm->program_state.name */
#define PGM_STATE_GET_T(name) pgm_state_getter_t(thrd, name)
#define PGM_STATE_GET(name) pgm_state_getter(pgm, name)

/*
** LOG MACROS
*/

#define BUF_LOG_LEN 256
#define TM_LOG(func, fmt, ...)                                       \
    do {                                                             \
        pthread_mutex_lock(&thrd->node->mtx_log);                    \
        char buf[BUF_LOG_LEN] = {0};                                 \
        time_t t = time(NULL);                                       \
        char *curr_time = asctime(localtime(&t));                    \
        uint32_t len = strlen(curr_time) - 5;                        \
                                                                     \
        strncpy(buf, curr_time, len);                                \
        snprintf(buf + len, BUF_LOG_LEN, "- [" func "] - " fmt "\n", \
                 __VA_ARGS__);                                       \
        write(thrd->node->tm_fd_log, buf, strlen(buf));              \
        pthread_mutex_unlock(&thrd->node->mtx_log);                  \
    } while (0)
#define TM_LOG2(func, fmt, ...)                                      \
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
           PGM_SPEC_GET_T(str_name), THRD_DATA_GET(uint32_t, pid),         \
           THRD_DATA_GET(pthread_t, tid),                                  \
           THRD_DATA_GET(int32_t, restart_counter));

#define TM_CHILDCONTROL_LOG(status)                                           \
    TM_LOG("child supervisor",                                                \
           "[%s pid[%d]] - tid[%lu] - restart_counter[%d] • [" status " %d]", \
           PGM_SPEC_GET_T(str_name), THRD_DATA_GET(uint32_t, pid),            \
           THRD_DATA_GET(pthread_t, tid),                                     \
           THRD_DATA_GET(int32_t, restart_counter), child_ret);

#define TM_STOP_LOG(status)                                                    \
    TM_LOG("stop timer",                                                       \
           "[%s pid[%d]] - tid[%lu] - rank[%d] - stop_time[%d sec] • [" status \
           "]",                                                                \
           PGM_SPEC_GET_T(str_name), THRD_DATA_GET(uint32_t, pid),             \
           THRD_DATA_GET(pthread_t, tid), THRD_DATA_GET(uint32_t, rid),        \
           PGM_SPEC_GET_T(stop_time));

#define TM_START_LOG(status)                                                 \
    TM_LOG(                                                                  \
        "start timer",                                                       \
        "[%s pid[%d]] - tid[%lu] - rank[%d] - start_time[%d sec] • [" status \
        "]",                                                                 \
        PGM_SPEC_GET_T(str_name), pid, THRD_DATA_GET(pthread_t, tid),        \
        THRD_DATA_GET(uint32_t, rid), PGM_SPEC_GET_T(start_time));

#endif
