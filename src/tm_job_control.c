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

static bool pgm_state_getter(struct program_specification *pgm,
                             enum pgm_states state) {
    bool b;

    pthread_mutex_lock(&pgm->mtx_pgm_state);
    b = *(uint8_t *)(&pgm->program_state) & state;
    pthread_mutex_unlock(&pgm->mtx_pgm_state);
    return b;
}

/* static bool pgm_state_getter_t(struct thread_data *thrd, */
/*                                enum pgm_states state) { */
/*     bool b; */

/*     pthread_mutex_lock(&thrd->pgm->mtx_pgm_state); */
/*     b = *(uint8_t *)(&thrd->pgm->program_state) & state; */
/*     pthread_mutex_unlock(&thrd->pgm->mtx_pgm_state); */
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
    THRD_DATA_SET(tid, 0);
    THRD_DATA_SET(exit, FALSE);
    THRD_DATA_SET(restart, FALSE);
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
    THRD_DATA_SET(restart, (rt > 0));
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

    gettimeofday(&start, NULL);
    THRD_DATA_SET(start_timestamp, start);
    THRD_DATA_SET(pid, pid);
    THRD_DATA_SET(restart_counter, rt);
    pthread_mutex_lock(&thrd->mtx_timer);
    pthread_cond_signal(&thrd->cond_timer);
    pthread_mutex_unlock(&thrd->mtx_timer);
    TM_THRD_LOG("LAUNCHED");
    debug_thrd();
}

static void *exit_launcher_thread(struct thread_data *thrd) {
    /* exit flag can be set & timer can be triggered by do_stop() */
    if (!THRD_DATA_GET(bool, exit)) {
        THRD_DATA_SET(exit, TRUE);
        pthread_mutex_lock(&thrd->mtx_timer);
        pthread_cond_signal(&thrd->cond_timer);
        pthread_mutex_unlock(&thrd->mtx_timer);
    }
    pthread_join(THRD_DATA_GET(pthread_t, timer_id), NULL);
    TM_THRD_LOG("EXITED");
    exit_thread(thrd);
    return NULL;
}

/*
 * checks if the process is stopped in the given time otherwise it sends a kill
 * signal. To be sure the signal is catched, it is sent in a loop during 1 sec
 * if ever the proc is still alive
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
    if (GET_PROC_EVENT == PROC_EV_RESTARTING) {
        THRD_DATA_SET(exit, FALSE);
        return 1;
    }
    return 0;
}

/*
 * This joinable thread is a timer which is coupled with its launcher thread.
 * It checks if the processus is launched correctly, then wait for a restart or
 * an exit, to count the stop time.
 */
static void *timer(void *arg) {
    struct thread_data *thrd = arg;
    struct timeval started;
    pid_t pid;

start_timer:
    SET_PROC_STATE(PROC_ST_STARTING);
    pthread_mutex_lock(&thrd->mtx_timer);
    sem_post(&thrd->sync); /* sync launcher thread with timer at init */
wait:
    if (THRD_DATA_GET(bool, exit)) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        if (stop_time(thrd))
            goto start_timer;
        else
            return NULL;
    }

    pthread_cond_wait(&thrd->cond_timer, &thrd->mtx_timer);
    pthread_mutex_unlock(&thrd->mtx_timer);

    if (THRD_DATA_GET(bool, exit)) {
        if (stop_time(thrd))
            goto start_timer;
        else
            return NULL;
    }
    THRD_DATA_SET(restart, FALSE);

    started = THRD_DATA_GET(tm_timeval_t, start_timestamp);
    pid = THRD_DATA_GET(uint32_t, pid);

    SET_PROC_STATE(PROC_ST_STARTING);
    while (timediff(&started) < PGM_SPEC_GET_T(start_time)) {
        if (THRD_DATA_GET(bool, exit)) {
            TM_START_LOG("EXITED BEFORE TIME TO LAUNCH");
            if (stop_time(thrd))
                goto start_timer;
            else
                return NULL;
        }
        if (THRD_DATA_GET(bool, restart)) {
            TM_START_LOG("DIDN'T LAUNCHED CORRECTLY");
            pthread_mutex_lock(&thrd->mtx_timer);
            goto wait;
        }
        usleep(START_SUPERVISOR_RATE);
    }
    SET_PROC_STATE(PROC_ST_STARTED);
    TM_START_LOG("LAUNCHED CORRECTLY");
    pthread_mutex_lock(&thrd->mtx_timer);
    goto wait;

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
    int32_t pgm_restart;
    pid_t pid;

    thrd->pgm->nb_thread_alive++;
    if (pthread_create(&thrd->timer_id, NULL, timer, arg))
        exit_thrd(thrd, "pthread_create() failed", __FILE__, __func__,
                  __LINE__);
start_launcher:
    sem_wait(&thrd->sync);
    pgm_restart = 1;
    THRD_DATA_SET(restart_counter, PGM_SPEC_GET_T(start_retries) + 1);
    SET_PROC_EVENT(PROC_EV_NOEVENT);

    while (pgm_restart > 0) {
        /* the more it restarts the more it sleeps (supervisord behavior) */
        sleep((PGM_SPEC_GET_T(start_retries) + 1) -
              THRD_DATA_GET(int32_t, restart_counter));

        pid = fork();
        if (pid == -1)
            exit_thrd(thrd, "fork() failed", __FILE__, __func__, __LINE__);
        if (pid == 0) {
            configure_and_launch(thrd);
            exit(EXIT_FAILURE);
        } else {
            thread_data_update(thrd, pid);
            child_control(thrd, pid);
            pgm_restart = PGM_SPEC_GET_T(e_auto_restart) *
                          (THRD_DATA_GET(int32_t, restart_counter));
        }
    }

    pthread_mutex_lock(&thrd->mtx_timer);
    if (GET_PROC_EVENT == PROC_EV_RESTARTING) {
        pthread_mutex_unlock(&thrd->mtx_timer);
        goto start_launcher;
    }
    pthread_mutex_unlock(&thrd->mtx_timer);
    return exit_launcher_thread(thrd);
}

/*
 * Start launcher_threads from one struct program_specification, in a
 * detached mode, if it doesn't exists yet and if is inactive.
 * If the thread already exists the restart_counter is still reset.
 **/
static uint8_t do_start(struct program_specification *pgm,
                        struct program_list *node) {
    struct thread_data *thrd;
    struct stat statbuf;

    if (stat(PGM_SPEC_GET(argv)[0], &statbuf) == -1)
        TM_LOG2("start error", "%s can't be executed", PGM_SPEC_GET(argv)[0]);

    TM_LOG2("start", "%s", PGM_SPEC_GET(str_name));
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        thrd = &pgm->thrd[id];

        if (THRD_DATA_GET(pthread_t, tid) == 0 && !IS_PROC_ACTIVE(thrd)) {
            if (pthread_create(&thrd->tid, &node->attr, routine_launcher_thrd,
                               thrd))
                log_error("failed to create launcher thread", __FILE__,
                          __func__, __LINE__);
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
        THRD_DATA_SET(restart_counter, 0);
        if (THRD_DATA_GET(pthread_t, tid) &&
            GET_PROC_STATE != PROC_ST_STOPPING) {
            /*
             * we signal the timer here because the kill above can miss or take
             * long time so the launcher_thread stay blocked and doesn't return
             * neither exit. The timer is responsible for kill with SIGTERM if
             * it takes too long time.
             */

            pthread_mutex_lock(&thrd->mtx_timer);
            THRD_DATA_SET(exit, TRUE);
            kill(THRD_DATA_GET(uint32_t, pid), pgm->stop_signal);
            pthread_cond_signal(&thrd->cond_timer);
            pthread_mutex_unlock(&thrd->mtx_timer);
        }
    }
    PGM_STATE_SET(need_to_stop, FALSE);
    return EXIT_SUCCESS;
}

static uint8_t do_nothing(struct program_specification *pgm,
                          struct program_list *node) {
    (void)pgm;
    (void)node;
    return EXIT_SUCCESS;
}

static uint8_t do_restart(struct program_specification *pgm,
                          struct program_list *node) {
    struct thread_data *thrd;

    TM_LOG2("restart", "%s", PGM_SPEC_GET(str_name));
    for (uint32_t i = 0; i < PGM_SPEC_GET(number_of_process); i++) {
        thrd = &pgm->thrd[i];
        SET_PROC_EVENT(PROC_EV_RESTARTING);
    }

    /*
     * As restarting event is set, active launcher_threads will stop but
     * instead of exiting will jump back to the init to start a new cycle
     */
    do_stop(pgm, node);
    /* This will start only the threads which aren't active yet */
    do_start(pgm, node);
    PGM_STATE_SET(need_to_restart, FALSE);
    return EXIT_SUCCESS;
}

static void init_handlers(s_client_handler *handler) {
    handler[CLIENT_NOTHING].cb = do_nothing;
    handler[CLIENT_START].cb = do_start;
    handler[CLIENT_RESTART].cb = do_restart;
    handler[CLIENT_STOP].cb = do_stop;
}

/*
 * Set need_to_start flag to programs which have their auto_start flag to true
 **/
static uint8_t set_autostart(struct program_list *node) {
    struct program_specification *pgm = node->program_linked_list;

    for (uint32_t i = 0; i < node->number_of_program; i++) {
        PGM_STATE_SET(need_to_start, pgm->auto_start);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

static void exit_job_control(struct program_list *node) {
    struct program_specification *pgm;
    uint32_t nb = 0;

    TM_LOG2("exit", "...", NULL);
    for (pgm = node->program_linked_list; pgm; pgm = pgm->next)
        nb += do_stop(pgm, node);
    (void)nb;
    for (int alive = 1; alive;) {
        alive = 0;
        for (pgm = node->program_linked_list; pgm; pgm = pgm->next)
            alive += (pgm->nb_thread_alive > 0);
        usleep(EXIT_MASTER_RATE);
    }
    for (int alive = 1; alive;) {
        alive = 0;
        for (pgm = node->program_linked_list; pgm; pgm = pgm->next)
            alive += (PGM_STATE_GET(started) > 0);
        usleep(EXIT_MASTER_RATE);
    }
}

/*
 * The master thread listen the client events - start, stop, restart - and
 * handle them.
 *
 * @args:
 *   void *arg  is the address of the struct program_list which is the node
 *              containing the program_specification linked list & metadata
 **/
static void *routine_master_thrd(void *arg) {
    struct program_list *node = arg;
    struct program_specification *pgm;
    s_client_handler handler[CLIENT_MAX_EVENT];
    uint8_t client_event;

    init_handlers(handler);
    set_autostart(node);
    while (node->exit == FALSE) {
        pgm = node->program_linked_list;
        while (pgm) {
            client_event = ((PGM_STATE_GET(need_to_restart) * CLIENT_RESTART) +
                            (PGM_STATE_GET(need_to_stop) * CLIENT_STOP) +
                            (PGM_STATE_GET(need_to_start) * CLIENT_START));
            if (client_event) handler[client_event].cb(pgm, node);
            pgm = pgm->next;
        }
        usleep(CLIENT_LISTENING_RATE);
    }
    exit_job_control(node);
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
