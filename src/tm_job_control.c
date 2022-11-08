#include "tm_job_control.h"

#include "taskmaster.h"

#define THRD_TYPE uint32_t
THRD_DATA_GET_IMPLEMENTATION
#undef THRD_TYPE
#define THRD_TYPE int32_t
THRD_DATA_GET_IMPLEMENTATION
#undef THRD_TYPE
#define THRD_TYPE bool
THRD_DATA_GET_IMPLEMENTATION
#undef THRD_TYPE
#define THRD_TYPE pthread_t
THRD_DATA_GET_IMPLEMENTATION
#undef THRD_TYPE
#define THRD_TYPE tm_timeval_t
THRD_DATA_GET_IMPLEMENTATION
#undef THRD_TYPE

/* getters */

/* static bool pgm_state_getter(struct program_specification *pgm, */
/*                              enum pgm_states state) { */
/*     bool b; */

/*     pthread_mutex_lock(&pgm->mtx_pgm_state); */
/*     b = *(uint8_t *)(&pgm->program_state) & state; */
/*     pthread_mutex_unlock(&pgm->mtx_pgm_state); */
/*     return b; */
/* } */

/* time diff in s */
static uint32_t timediff(struct timeval *time1) {
    struct timeval time2;

    gettimeofday(&time2, NULL);
    return (((time2.tv_sec - time1->tv_sec) * 1000000) +
            (time2.tv_usec - time1->tv_usec)) /
           1000000;
}

/* Set pid & tid of the related thread_data structure to 0 */
static void exit_thread(struct thread_data *thrd) {
    if (!thrd) return;
    THRD_DATA_SET(pid, 0);
    THRD_DATA_SET(proc_restart, FALSE);
    thrd->pgm->nb_thread_alive--;
    if (PGM_SPEC_GET_T(nb_thread_alive) == 0) PGM_STATE_SET_T(started, FALSE);
}

/*
 * wait for child to exit() or to be killed by any signal
 **/
static int32_t child_control(struct thread_data *thrd, pid_t pid) {
    int32_t rt, w, wstatus, child_ret = 0;
    uint8_t expected = FALSE;

    do {
        w = waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
        if (w == -1)
            log_error("waitpid() failed", __FILE__, __func__, __LINE__);
        if (WIFEXITED(wstatus)) {
            child_ret = WEXITSTATUS(wstatus);
            for (uint16_t i = 0;
                 !expected && i < PGM_SPEC_GET_T(exit_codes_number); i++)
                expected = child_ret == PGM_SPEC_GET_T(exit_codes)[i];
            if (expected) {
                if (PGM_SPEC_GET_T(e_auto_restart) ==
                    PROGRAM_AUTO_RESTART_UNEXPECTED)
                    THRD_DATA_SET(restart_counter, 0);
                TM_CHILDCONTROL_LOG("EXITED WITH EXPECTED STATUS");
            } else
                TM_CHILDCONTROL_LOG("EXITED WITH UNEXPECTED STATUS");
        } else if (WIFSIGNALED(wstatus)) {
            child_ret = WTERMSIG(wstatus);
            TM_CHILDCONTROL_LOG("KILLED BY SIGNAL");
        } else if (WIFSTOPPED(wstatus)) {
            child_ret = WSTOPSIG(wstatus);
            TM_CHILDCONTROL_LOG("STOPPED BY SIGNAL");
        } else if (WIFCONTINUED(wstatus)) {
            TM_CHILDCONTROL_LOG("CONTINUED");
        }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    rt = THRD_DATA_GET(int32_t, restart_counter) - 1;
    THRD_DATA_SET(proc_restart, (rt > 0));
    THRD_DATA_SET(pid, 0);
    return child_ret;
}

/*
 * Sets configuration asked from config file like umask, working directory
 * or file logging.
 * Then execve() the process.
 **/
static void configure_and_launch(struct thread_data *thrd) {
    struct program_specification *pgm = thrd->pgm;
    struct stat statbuf;

    if (stat(PGM_SPEC_GET(argv)[0], &statbuf) == -1) {
        TM_LOG("start error", "%s can't be executed", PGM_SPEC_GET(argv)[0]);
        sleep(1);
        exit(EXIT_FAILURE);
    }
    if (pgm->umask) umask(pgm->umask); /* default file mode creation */
    /* privilege de-escalation if launched in sudo mode */
    if (getuid() == 0)
        if (setuid(atoi(getenv("SUDO_UID"))) == -1)
            TM_LOG("setuid()", "pid [%d] - failed to set uid to %d", getpid(),
                   atoi(getenv("SUDO_UID")));
    if (pgm->working_dir) {
        if (chdir((char *)pgm->working_dir) == -1)
            TM_LOG("chdir()", "pid [%d] - failed to change directory to %s",
                   getpid(), PGM_SPEC_GET_T(working_dir));
    }
    if (pgm->log.out != UNINITIALIZED_FD) dup2(pgm->log.out, STDOUT_FILENO);
    if (pgm->log.err != UNINITIALIZED_FD) dup2(pgm->log.err, STDERR_FILENO);
    if (execve(pgm->argv[0], pgm->argv, (char **)pgm->env) == -1) {
        TM_LOG("execve()", "pid [%d] - failed to call processus %s", getpid(),
               PGM_SPEC_GET_T(str_name));
        err_display("execve() failed", __FILE__, __func__, __LINE__);
    }
    exit(EXIT_FAILURE);
}

/*
 * Update information of the thread_data struct related to one process - the
 * timestamp, pid & restart_counter.
 * Launch an attached timer thread which monitor if the process is still alive
 * after 'start_time' seconds.
 **/
static void thread_data_update(struct thread_data *thrd, pid_t pid) {
    int32_t rt = THRD_DATA_GET(int32_t, restart_counter) - 1;
    struct timeval start;

    /* if the stop timer didn't finished, wait it */
    pthread_mutex_lock(&thrd->mtx_timer);
    gettimeofday(&start, NULL);
    THRD_DATA_SET(start_timestamp, start);
    THRD_DATA_SET(pid, pid);
    THRD_DATA_SET(restart_counter, rt);
    pthread_cond_signal(&thrd->cond_timer); /* start the start timer */
    pthread_mutex_unlock(&thrd->mtx_timer);
    TM_THRD_LOG("LAUNCHED");
    debug_thrd();
}

static void *exit_launcher_thread(struct thread_data *thrd) {
    if (pthread_join(THRD_DATA_GET(pthread_t, timer_id), NULL))
        err_display("pthread_join() failed", __FILE__, __func__, __LINE__);
    exit_thread(thrd);
    TM_THRD_LOG("EXITED");
    return NULL;
}

/*
 * checks if the process is stopped in the given time otherwise it sends a kill
 * signal. To be sure the signal is catched, it is sent in a loop during 1 sec
 * if ever the proc is still alive.
 * return 0 if LT & timer have to exit (exit flag) otherwise returns non-NULL
 * (if restarted or killed).
 */
static uint8_t stop_time(struct thread_data *thrd) {
    struct timeval stop;

    SET_PROC_STATE(PROC_ST_STOPPING);
    while ((THRD_DATA_GET(uint32_t, pid) > 0) &&
           timediff(&PGM_SPEC_GET_T(stop_timestamp)) <
               PGM_SPEC_GET_T(stop_time))
        usleep(STOP_SUPERVISOR_RATE);

    gettimeofday(&stop, NULL);
    if (THRD_DATA_GET(uint32_t, pid)) {
        THRD_DATA_SET(restart_counter, 0);
        while (THRD_DATA_GET(uint32_t, pid) &&
               timediff(&stop) < KILL_TIME_LIMIT) {
            kill(THRD_DATA_GET(uint32_t, pid), SIGTERM);
            usleep(STOP_SUPERVISOR_RATE);
        }
        if (timediff(&stop) > KILL_TIME_LIMIT) {
            TM_STOP_LOG("ERR: TASKMASTER DIDN'T SUCCEEDED TO KILL THE PROC");
        } else
            TM_STOP_LOG("PROCESSUS HAD BEEN KILLED");
    } else
        TM_STOP_LOG("PROCESSUS STOPPED AS EXPECTED");

    SET_PROC_STATE(PROC_ST_STOPPED);
    return EXIT_SUCCESS;
}

/*
 * This joinable thread is a timer which is coupled with its launcher thread.
 * It checks if the processus is launched correctly, then wait for a restart or
 * an exit, to count the stop time.
 */
static void *timer(void *arg) {
    struct thread_data *thrd = arg;
    struct timeval started;
    bool init = FALSE;
    pid_t pid;

idle_timer:
    pthread_barrier_wait(&thrd->sync_barrier);
    pthread_mutex_lock(&thrd->mtx_wakeup);
    if (!init) {
        init = TRUE;
        /* sync thread_pool creation with start*/
        sem_post(&thrd->sync);
    }
    /* Waits for MT to signal a start */
    pthread_cond_wait(&thrd->cond_wakeup, &thrd->mtx_wakeup);
    pthread_mutex_unlock(&thrd->mtx_wakeup);
    if (GET_THRD_EVENT == THRD_EV_EXIT) return NULL;
start_timer:
    pthread_barrier_wait(&thrd->sync_barrier);
    pthread_mutex_lock(&thrd->mtx_timer);
    SET_PROC_STATE(PROC_ST_STARTING); /* Careful: this clears thread_event */
    sem_post(&thrd->sync); /* sync launcher thread with timer at init */
wait:
    if (GET_THRD_EVENT) {
        stop_time(thrd);
        goto check_ret;
    }

    pthread_cond_wait(&thrd->cond_timer, &thrd->mtx_timer);

    if (GET_THRD_EVENT) {
        stop_time(thrd);
        goto check_ret;
    }
    THRD_DATA_SET(proc_restart, FALSE);

    started = THRD_DATA_GET(tm_timeval_t, start_timestamp);
    pid = THRD_DATA_GET(uint32_t, pid);

    SET_PROC_STATE(PROC_ST_STARTING);
    while (timediff(&started) < PGM_SPEC_GET_T(start_time)) {
        if (GET_THRD_EVENT) {
            TM_START_LOG("EXITED BEFORE TIME TO LAUNCH");
            stop_time(thrd);
            goto check_ret;
        }
        if (THRD_DATA_GET(bool, proc_restart)) {
            TM_START_LOG("DIDN'T LAUNCHED CORRECTLY");
            goto wait;
        }
        usleep(START_SUPERVISOR_RATE);
    }
    SET_PROC_STATE(PROC_ST_STARTED);
    TM_START_LOG("LAUNCHED CORRECTLY");
    goto wait;

check_ret:
    if (GET_THRD_EVENT == THRD_EV_NOEVENT || GET_THRD_EVENT == THRD_EV_STOP) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto idle_timer;
    } else if (GET_THRD_EVENT == THRD_EV_RESTART) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto start_timer;
    }
    pthread_mutex_unlock(&thrd->mtx_timer);
    return NULL;
}

static void *run_process(struct thread_data *thrd) {
    int32_t pgm_restart = 1;
    pid_t pid;

    THRD_DATA_SET(restart_counter, PGM_SPEC_GET_T(start_retries) + 1);
    while (pgm_restart > 0) {
        /* the more it restarts the more it sleeps (supervisord behavior) */
        sleep((PGM_SPEC_GET_T(start_retries) + 1) -
              THRD_DATA_GET(int32_t, restart_counter));

        if (GET_THRD_EVENT) break;
        pid = fork();
        if (pid == -1)
            exit_thrd(thrd, "fork() failed", __FILE__, __func__, __LINE__);
        if (pid == 0)
            configure_and_launch(thrd);
        else {
            thread_data_update(thrd, pid);
            child_control(thrd, pid);
            pgm_restart = PGM_SPEC_GET_T(e_auto_restart) *
                          (THRD_DATA_GET(int32_t, restart_counter));
        }
    }
    return NULL;
}

/*
 * Routine of a launcher_thread.
 * The thread must first find its id to match with the struct thread_data
 * it is allowed to write/read in.
 * Redirect stdout and stderr of child if necessary before execve()
 * Responsible for relaunching the program if required.
 * @args:
 *   void *arg  is the address of one struct program_specification which
 *              contains the configuration of the related program with, among
 *              other things, array of struct thread_data.
 **/
static void *routine_launcher_thrd(void *arg) {
    struct thread_data *thrd = arg;
    bool init = FALSE;

    thrd->pgm->nb_thread_alive++;
    if (pthread_create(&thrd->timer_id, NULL, timer, arg))
        exit_thrd(thrd, "pthread_create() failed", __FILE__, __func__,
                  __LINE__);
idle_launcher:
    pthread_barrier_wait(&thrd->sync_barrier);
    pthread_mutex_lock(&thrd->mtx_wakeup);
    if (!init) {
        init = TRUE;
        /* sync thread_pool creation with start*/
        sem_post(&thrd->sync);
    }
    /* Waits for MT to signal a start */
    pthread_cond_wait(&thrd->cond_wakeup, &thrd->mtx_wakeup);
    pthread_mutex_unlock(&thrd->mtx_wakeup);
    if (GET_THRD_EVENT == THRD_EV_EXIT) return exit_launcher_thread(thrd);
start_launcher:
    pthread_barrier_wait(&thrd->sync_barrier);
    sem_wait(&thrd->sync);
    /* thrd_event may have changed in the meantime so we need to check it */
    if (GET_THRD_EVENT == THRD_EV_EXIT) return exit_launcher_thread(thrd);

    run_process(thrd);

    pthread_mutex_lock(&thrd->mtx_timer); /* wait stop timer to finish */
    if (GET_THRD_EVENT == THRD_EV_NOEVENT) {
        SET_THRD_EVENT(THRD_EV_STOP);
        pthread_cond_signal(&thrd->cond_timer);
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto idle_launcher;
    } else if (GET_THRD_EVENT == THRD_EV_STOP) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto idle_launcher;
    } else if (GET_THRD_EVENT == THRD_EV_RESTART) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto start_launcher;
    } else { /* PROC_EV_EXITING */
        pthread_mutex_unlock(&thrd->mtx_timer);
        return exit_launcher_thread(thrd);
    }
}

static void stop_signal(struct thread_data *thrd, int32_t signal) {
    THRD_DATA_SET(restart_counter, 0);
    if (THRD_DATA_GET(pthread_t, tid) && GET_PROC_STATE != PROC_ST_STOPPING) {
        /*
         * we signal the timer here because the kill above can miss or take
         * long time so the launcher_thread stay blocked and doesn't return
         * neither exit. The timer is responsible for kill with SIGTERM if
         * it takes too long time.
         */
        pthread_mutex_lock(&thrd->mtx_timer);
        pthread_cond_signal(&thrd->cond_timer);
        if (THRD_DATA_GET(uint32_t, pid))
            kill(THRD_DATA_GET(uint32_t, pid), signal);
        pthread_mutex_unlock(&thrd->mtx_timer);
    }
}

static uint8_t exit_pgm_launchers(struct program_specification *pgm,
                                  struct program_list *node) {
    struct thread_data *thrd;
    struct timeval stop;

    gettimeofday(&stop, NULL);
    PGM_SPEC_SET(stop_timestamp, stop);
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];
        SET_THRD_EVENT(THRD_EV_EXIT);
        stop_signal(thrd, pgm->stop_signal);

        if (THRD_DATA_GET(pthread_t, tid) && !IS_PROC_ACTIVE(thrd)) {
            pthread_mutex_lock(&thrd->mtx_wakeup);
            pthread_cond_broadcast(&thrd->cond_wakeup);
            pthread_mutex_unlock(&thrd->mtx_wakeup);
        }
    }
    return EXIT_SUCCESS;
}

static uint8_t join_pgm_launchers(struct program_specification *pgm) {
    struct thread_data *thrd;

    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];
        if (pthread_join(THRD_DATA_GET(pthread_t, tid), NULL)) {
            perror("oops");
            err_display("pthread_join() failed", __FILE__, __func__, __LINE__);
        }
    }
    return EXIT_SUCCESS;
}

static uint8_t create_launcher_pool(struct program_specification *pgm) {
    struct thread_data *thrd;
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];
        if (pthread_create(&thrd->tid, NULL, routine_launcher_thrd, thrd))
            log_error("failed to create launcher thread", __FILE__, __func__,
                      __LINE__);

        /* waits that LT & its timer are idle and waiting for start signal*/
        sem_wait(&thrd->sync);
        sem_wait(&thrd->sync);
    }
    return EXIT_SUCCESS;
}

static uint8_t do_nothing(struct program_specification *pgm,
                          struct program_list *node) {
    (void)pgm;
    (void)node;
    return EXIT_SUCCESS;
}

/*
 * Start launcher_threads from one struct program_specification, in a
 * detached mode, if it doesn't exists yet and if is inactive.
 * If the thread already exists the restart_counter is still reset.
 **/
static uint8_t do_start(struct program_specification *pgm,
                        struct program_list *node) {
    struct thread_data *thrd;

    TM_LOG2("start", "%s", PGM_SPEC_GET(str_name));
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];

        if (THRD_DATA_GET(pthread_t, tid) && !IS_PROC_ACTIVE(thrd)) {
            pthread_mutex_lock(&thrd->mtx_wakeup);
            /* signal LT and its timer to start workflow */
            pthread_cond_broadcast(&thrd->cond_wakeup);
            pthread_mutex_unlock(&thrd->mtx_wakeup);
        }
    }
    PGM_STATE_SET(need_to_start, FALSE);
    return EXIT_SUCCESS;
}

/*
 * Stops all processus from one struct program_specification with the signal
 * set in configuration file.
 * Does nothing if the thread is already down or is stopping.
 **/
static uint8_t do_stop(struct program_specification *pgm,
                       struct program_list *node) {
    struct thread_data *thrd;
    struct timeval stop;

    gettimeofday(&stop, NULL);
    PGM_SPEC_SET(stop_timestamp, stop);
    TM_LOG2("stop", "%s", PGM_SPEC_GET(str_name));
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];
        SET_THRD_EVENT(THRD_EV_STOP);
        stop_signal(thrd, pgm->stop_signal);
    }
    PGM_STATE_SET(need_to_stop, FALSE);
    return EXIT_SUCCESS;
}

static uint8_t do_restart(struct program_specification *pgm,
                          struct program_list *node) {
    struct thread_data *thrd;
    struct timeval stop;

    gettimeofday(&stop, NULL);
    PGM_SPEC_SET(stop_timestamp, stop);
    TM_LOG2("restart", "%s", PGM_SPEC_GET(str_name));
    for (uint32_t id = 0; id < PGM_SPEC_GET(number_of_process); id++) {
        thrd = &pgm->thrd[id];
        SET_THRD_EVENT(THRD_EV_RESTART);
        stop_signal(thrd, pgm->stop_signal);

        if (THRD_DATA_GET(pthread_t, tid) && !IS_PROC_ACTIVE(thrd)) {
            pthread_mutex_lock(&thrd->mtx_wakeup);
            pthread_cond_broadcast(&thrd->cond_wakeup);
            pthread_mutex_unlock(&thrd->mtx_wakeup);
        }
    }
    PGM_STATE_SET(need_to_restart, FALSE);
    return EXIT_SUCCESS;
}

/* exit and destroy pgm. This function is blocking as it joins launchers */
static uint8_t do_del(struct program_specification *pgm,
                      struct program_list *node) {
    TM_LOG2("delete", "%s", PGM_SPEC_GET(str_name));
    exit_pgm_launchers(pgm, node);
    join_pgm_launchers(pgm);

    free_program_specification(pgm);

    return EXIT_SUCCESS;
}

/* create launchers of pgm and start them if auto_start is true */
static uint8_t do_add(struct program_specification *pgm,
                      struct program_list *node) {
    TM_LOG2("add", "%s", PGM_SPEC_GET(str_name));
    create_launcher_pool(pgm);

    if (PGM_SPEC_GET(auto_start)) do_start(pgm, node);
    return EXIT_SUCCESS;
}

/* copy new_pgm values into old_pgm & destroy new_pgm */
static uint8_t do_soft_reload(struct program_specification *pgm,
                              struct program_list *node) {
    struct program_specification *new = pgm->restart_tmp_program;

    TM_LOG2("soft reload", "%s", PGM_SPEC_GET(str_name));
    /* soft copy from new to pgm */
    pgm->auto_start = new->auto_start;
    pgm->e_auto_restart = new->e_auto_restart;
    pgm->start_time = new->start_time;
    pgm->start_retries = new->start_retries;
    pgm->stop_signal = new->stop_signal;
    pgm->stop_time = new->stop_time;
    for (uint32_t i = 0; i < pgm->exit_codes_number; i++)
        pgm->exit_codes[i] = new->exit_codes[i];

    free_program_specification(new);

    return EXIT_SUCCESS;
}

/*
 * insert new pgm into the old list, exit & destroy pgm, start new pgm.
 * Blocking
 */
static uint8_t do_hard_reload(struct program_specification *pgm,
                              struct program_list *node) {
    struct program_specification *new = pgm->restart_tmp_program,
                                 *prev = new->restart_tmp_program;

    TM_LOG2("hard reload", "%s", PGM_SPEC_GET(str_name));

    /* insert new in the old list. new->restart_tmp_program holds pgm prev */
    new->next = pgm->next;
    if (node->program_linked_list == pgm)
        node->program_linked_list = new;
    else
        prev->next = new;
    if (node->last_program_linked_list == pgm)
        node->last_program_linked_list = new;

    new->restart_tmp_program = NULL;
    pgm->restart_tmp_program = NULL;

    do_del(pgm, node);
    do_add(new, node);

    return EXIT_SUCCESS;
}

/* exit LT and wait them. */
static uint8_t do_exit(struct program_specification *pgm_head,
                       struct program_list *node) {
    node->exit = TRUE;
    TM_LOG2("exit", "...", NULL);
    for (struct program_specification *pgm = pgm_head; pgm; pgm = pgm->next)
        exit_pgm_launchers(pgm, node);
    for (struct program_specification *pgm = pgm_head; pgm; pgm = pgm->next)
        join_pgm_launchers(pgm);
    return EXIT_SUCCESS;
}

static uint8_t create_thread_pool(struct program_list *node) {
    struct program_specification *pgm = node->program_linked_list;

    for (uint32_t i = 0; i < node->number_of_program && pgm; i++) {
        create_launcher_pool(pgm);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

static uint8_t set_autostart(struct program_list *node) {
    struct program_specification *pgm = node->program_linked_list;

    for (uint32_t i = 0; i < node->number_of_program && pgm; i++) {
        if (PGM_SPEC_GET(auto_start)) do_start(pgm, node);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

uint8_t (*execute_event[CLIENT_MAX_EVENT])(struct program_specification *,
                                           struct program_list *) = {
    do_nothing,     do_start,       do_restart, do_stop, do_exit,
    do_soft_reload, do_hard_reload, do_add,     do_del,
};

/*
 * The master thread listen the client events
 * - start, stop, restart, reload, exit - and handle them.
 *
 * @args:
 *   void *arg  is the address of the struct program_list which is the node
 *              containing the program_specification linked list & metadata
 **/
static void *routine_master_thrd(void *arg) {
    struct program_list *node = arg;
    struct s_event client_ev;

    create_thread_pool(node);
    set_autostart(node);

    while (node->exit == FALSE) {
        sem_wait(&node->new_event);
        pthread_mutex_lock(&node->mtx_queue);
        client_ev = node->event_queue[0];
        for (uint32_t i = 0; i < node->ev_queue_size; i++) {
            node->event_queue[i] = node->event_queue[i + 1];
        }
        node->ev_queue_size--;
        pthread_mutex_unlock(&node->mtx_queue);
        sem_post(&node->free_place);
        execute_event[client_ev.type](client_ev.pgm, node);
    }
    TM_LOG2("taskmaster", "program exit", NULL);
    return NULL;
}

/*
 * Sets each *thrd variable into the program_specification linked list &
 * launch the master thread which controls and launch all others
 * launcher_threads
 * @args:
 *   struct program_list *pgm_node  address of a node which contains
 *                                  program_specification linked list & metadata
 **/
uint8_t tm_job_control(struct program_list *node) {
    if (pthread_create(&node->master_thread, NULL, routine_master_thrd, node))
        log_error("failed to create master thread", __FILE__, __func__,
                  __LINE__);
    TM_LOG2("taskmaster", "program started", NULL);
    return EXIT_SUCCESS;
}
