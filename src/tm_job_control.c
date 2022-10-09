#include "tm_job_control.h"

#include "taskmaster.h"

/* time diff in s */
static uint32_t timediff(struct timeval *time1) {
    struct timeval time2;

    gettimeofday(&time2, NULL);
    return (((time2.tv_sec - time1->tv_sec) * 1000000) +
            (time2.tv_usec - time1->tv_usec)) /
           1000000;
}

/* Set pid & tid of the related thread_data structure to 0 */
static void exit_thread(struct program_specification *pgm, int32_t id) {
    if (!pgm || id == -1) return;
    THRD_DATA_SET(pid, 0);
    THRD_DATA_SET(tid, 0);
    pgm->nb_thread_alive -= 1;
}

static int32_t get_id(const struct program_specification *pgm) {
    pthread_t tid = pthread_self();
    for (uint32_t i = 0; i < pgm->number_of_process; i++) {
        if (tid == pgm->thrd[i].tid && i == pgm->thrd[i].rid) return i;
    }
    return -1;
}

/*
 * wait for child to exit() or to be killed by any signal
 **/
static int32_t child_control(const struct program_specification *pgm,
                             int32_t id, pid_t pid) {
    struct program_list *node = pgm->node;
    int32_t w, wstatus, child_ret = 0;
    uint8_t expected = FALSE;

    do {
        w = waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
        if (w == -1)
            log_error("waitpid() failed", __FILE__, __func__, __LINE__);
        if (WIFEXITED(wstatus)) {
            child_ret = WEXITSTATUS(wstatus);
            THRD_DATA_SET(exit_status, child_ret);
            for (uint16_t i = 0;
                 !expected && i < PGM_SPEC_GET(exit_codes_number); i++)
                expected = child_ret == PGM_SPEC_GET(exit_codes)[i];
            if (expected) {
                if (PGM_SPEC_GET(e_auto_restart) ==
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
    return child_ret;
}

/*
 * Sets configuration asked from config file like umask, working directory
 * or file logging.
 * Then execve() the process.
 **/
static void configure_and_launch(struct program_list *node,
                                 struct program_specification *pgm) {
    if (pgm->umask) umask(pgm->umask);
    if (pgm->working_dir) {
        if (chdir((char *)pgm->working_dir) == -1)
            TM_LOG("chdir()", "pid [%d] - failed to change directory to %s",
                   getpid(), PGM_SPEC_GET(working_dir));
    }
    if (pgm->log.out != UNINITIALIZED_FD) dup2(pgm->log.out, STDOUT_FILENO);
    if (pgm->log.err != UNINITIALIZED_FD) dup2(pgm->log.err, STDERR_FILENO);
    if (execve(pgm->argv[0], pgm->argv, (char **)pgm->env) == -1) {
        TM_LOG("execve()", "pid [%d] - failed to change call processus %s",
               getpid(), PGM_SPEC_GET(str_name));
        err_display("execve() failed", __FILE__, __func__, __LINE__);
    }
}

static void thread_data_update(struct program_list *node,
                               struct program_specification *pgm, int32_t id,
                               pid_t pid) {
    struct timeval start;

    gettimeofday(&start, NULL);
    THRD_DATA_SET(start_timestamp, start);
    THRD_DATA_SET(pid, pid);
    debug_thrd();
    THRD_DATA_SET(restart_counter, THRD_DATA_GET(restart_counter) - 1);
    TM_THRD_LOG("LAUNCHED");
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
    struct program_specification *pgm = arg;
    struct program_list *node = pgm->node;
    int32_t id = get_id(pgm), pgm_restart = 1;
    pid_t pid;

    pgm->nb_thread_alive += 1;
    if (id == -1)
        exit_thrd(NULL, id, "couldn't find thread id", __FILE__, __func__,
                  __LINE__);
    while (pgm_restart >= 0) {
        pid = fork();
        if (pid == -1)
            exit_thrd(pgm, id, "fork() failed", __FILE__, __func__, __LINE__);
        if (pid == 0) {
            configure_and_launch(node, pgm);
            exit(EXIT_FAILURE);
        } else {
            thread_data_update(node, pgm, id, pid);
            child_control(pgm, id, pid);
            pgm_restart =
                pgm->e_auto_restart * (THRD_DATA_GET(restart_counter));
        }
    }
    TM_THRD_LOG("EXITED");
    exit_thread(pgm, id);
    return NULL;
}

/*
 * Creates all launcher_threads from one struct program_specification, in a
 * detached mode
 * If the thread already exists the restart_counter is still reset.
 **/
static uint8_t create_launcher_threads(struct program_list *node,
                                       struct program_specification *pgm) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(restart_counter, PGM_SPEC_GET(start_retries));
        if (THRD_DATA_GET(tid) == 0 && THRD_DATA_GET(pid) == 0) {
            THRD_DATA_SET(exit_status, 0);
            if (pthread_create(&pgm->thrd[id].tid, &node->attr,
                               routine_launcher_thrd, pgm))
                log_error("failed to create launcher thread", __FILE__,
                          __func__, __LINE__);
        }
    }
    return EXIT_SUCCESS;
}

/*
 * wrapper around create_launcher_thread() to loop thru the
 * program_specification linked list
 **/
static uint8_t launch_all(struct program_list *node) {
    struct program_specification *pgm = node->program_linked_list;

    for (uint32_t i = 0; i < node->number_of_program; i++) {
        if (pgm->auto_start) {
            PGM_STATE_SET(started, TRUE);
            if (create_launcher_threads(node, pgm)) return EXIT_FAILURE;
        }
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

/*
 * Stops all processus from one struct program_specification with the signal
 * set in configuration file.
 * Does nothing if the thread is already down.
 **/
static uint8_t stop_all_processus(struct program_specification *pgm,
                                  struct program_list *node) {
    struct timeval stop;

    gettimeofday(&stop, NULL);
    PGM_SPEC_SET(stop_timestamp, stop);
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(restart_counter, -1);
        if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) {
            TM_LOG("stop_all_processus()",
                   "pgm [%s] - pid [%d] - stop asked with signal %d",
                   PGM_SPEC_GET(str_name), THRD_DATA_GET(pid),
                   PGM_SPEC_GET(stop_signal));
            kill(THRD_DATA_GET(pid), pgm->stop_signal);
        }
    }
    return EXIT_SUCCESS;
}

/*
 * Kills all processus from one struct program_specification with SIGTERM
 * Does nothing if the thread is already down.
 **/
static uint8_t kill_all_processus(struct program_specification *pgm,
                                  struct program_list *node) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(restart_counter, 0);
        if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) {
            TM_LOG("kill_all_processus()",
                   "pgm [%s] - pid [%d] - kill with signal %d",
                   PGM_SPEC_GET(str_name), THRD_DATA_GET(pid), 9);
            kill(THRD_DATA_GET(pid), SIGTERM);
        }
    }
    return EXIT_SUCCESS;
}

static uint8_t do_nothing(struct program_specification *pgm,
                          struct program_list *node) {
    (void)pgm;
    (void)node;
    return EXIT_SUCCESS;
}

/* To be short, basically waits for pgm->start_time to be elapsed then checks
 * if programs are running as expected.
 **/
static void start_time_watcher(const struct program_specification *pgm,
                               struct program_list *node, s_timewatch *ptr,
                               uint16_t nb_started) {
    int id, ok = 1;

    while (ok) {
        ok = 0;
        for (uint32_t i = 0; i < nb_started; i++) {
            if (ptr[i].ok) continue;
            id = ptr[i].rid;
            if (ptr[id].tid != THRD_DATA_GET(tid)) {
                ptr[id].ok = TRUE;
                TM_LOG("start time watcher", "processus nb [%d] exited to soon",
                       id);
            } else if (THRD_DATA_GET(restart_counter) <
                       PGM_SPEC_GET(start_retries) - 1) {
                ptr[id].ok = TRUE;
                TM_START_LOG("DIDN'T LAUNCHED CORRECTLY");
            } else if (timediff(&THRD_DATA_GET(start_timestamp)) >
                       PGM_SPEC_GET(start_time)) {
                ptr[id].ok = TRUE;
                TM_START_LOG("LAUNCHED CORRECTLY");
            } else
                ok++;
        }
        usleep(500);
    }
}

/*
 * start threads which aren't already started. Set program state.
 * Check if all new launcher_thread have they restart_counter decremented which
 * means it has effectively forked() and execve() - in an amount of time given
 * from the config file
 **/
static uint8_t do_start(struct program_specification *pgm,
                        struct program_list *node) {
    s_timewatch *ptr;
    uint16_t nb_started = 0;

    PGM_STATE_SET(need_to_start, FALSE);
    PGM_STATE_SET(starting, TRUE);
    TM_LOG("start", "%s", PGM_SPEC_GET(str_name));
    pthread_mutex_unlock(&pgm->mtx_client_event);
    create_launcher_threads(node, pgm);

    for (uint32_t id = 0; id < PGM_SPEC_GET(number_of_process); id++) {
        if (timediff(&THRD_DATA_GET(start_timestamp)) < 2) nb_started++;
    }
    ptr = calloc(nb_started, sizeof(*ptr));
    if (!ptr && nb_started)
        log_error("failed to calloc() anon struct", __FILE__, __func__,
                  __LINE__);
    for (uint32_t id = 0; id < PGM_SPEC_GET(number_of_process); id++) {
        if (timediff(&THRD_DATA_GET(start_timestamp)) < 2) {
            ptr->rid = THRD_DATA_GET(rid);
            ptr->tid = THRD_DATA_GET(tid);
        }
    }

    PGM_STATE_SET(started, TRUE);
    PGM_STATE_SET(starting, FALSE);
    PGM_STATE_SET(restarting, FALSE);

    start_time_watcher(pgm, node, ptr, nb_started);

    free(ptr);
    return EXIT_SUCCESS;
}

/*
 * Stop threads which are alive. Update program state. Check if processus
 * stopped quickly enough according to config file and kill them if not.
 **/
static uint8_t do_stop(struct program_specification *pgm,
                       struct program_list *node) {
    struct timeval stoptime;
    uint8_t alive = TRUE;

    PGM_STATE_SET(stopping, TRUE);
    PGM_STATE_SET(need_to_stop, FALSE);
    TM_LOG("stop", "%s", PGM_SPEC_GET(str_name));
    pthread_mutex_unlock(&pgm->mtx_client_event);
    stop_all_processus(pgm, node);
    gettimeofday(&stoptime, NULL);

    /* check if the processes are stopped in the given time */
    while (alive && timediff(&stoptime) < PGM_SPEC_GET(stop_time)) {
        // TODO: PK ça rentre pas la dedans
        usleep(100);
        alive = FALSE;
        for (uint32_t id = 0; id < PGM_SPEC_GET(number_of_process); id++)
            if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) alive = TRUE;
    }
    if (alive) {
        kill_all_processus(pgm, node);
        TM_LOG("do_stop()", "All %s processus had been killed", pgm->str_name);
    } else
        TM_LOG("stop command", "All %s processus stopped as expected",
               pgm->str_name);

    PGM_STATE_SET(stopping, FALSE);
    PGM_STATE_SET(started, FALSE);
    return EXIT_SUCCESS;
}

static uint8_t do_restart(struct program_specification *pgm,
                          struct program_list *node) {
    PGM_STATE_SET(need_to_restart, FALSE);
    PGM_STATE_SET(restarting, TRUE);
    TM_LOG("restart", "%s", PGM_SPEC_GET(str_name));
    do_stop(pgm, node); /* unlock */
    pthread_mutex_lock(&pgm->mtx_client_event);
    do_start(pgm, node); /* unlock */
    return EXIT_SUCCESS;
}

static void *routine_client_event(void *arg) {
    s_client_handler *handler = arg;

    handler->cb(handler->pgm, handler->node);
    return NULL;
}

static int8_t handle_event(s_client_handler *handler,
                           struct program_specification *pgm,
                           struct program_list *node, uint8_t client_event) {
    pthread_t tid;

    pthread_mutex_lock(&pgm->mtx_client_event);
    if (PGM_STATE_GET(starting) || PGM_STATE_GET(stopping) ||
        PGM_STATE_GET(restarting)) {
        pthread_mutex_unlock(&pgm->mtx_client_event);
        return EXIT_FAILURE;
    }
    handler[client_event].pgm = pgm;
    handler[client_event].node = node;
    if (pthread_create(&tid, &node->attr, routine_client_event,
                       &handler[client_event])) {
        log_error("failed to create client_event thread", __FILE__, __func__,
                  __LINE__);
        pthread_mutex_unlock(&pgm->mtx_client_event);
    }
    return EXIT_SUCCESS;
}

static void init_handlers(s_client_handler *handler) {
    handler[CLIENT_NOTHING].cb = do_nothing;
    handler[CLIENT_START].cb = do_start;
    handler[CLIENT_RESTART].cb = do_restart;
    handler[CLIENT_STOP].cb = do_stop;
}

/*
 * This is the routine of the master thread. It begins with launching all
 * launcher_threads (those who execve then monitor there child) for all programs
 *
 * It's also responsible for killing or restarting all thread from a program in
 * case of config reload. Or just kill a program directly.
 * It monitors the program_specification linked list to see whether the client
 * asks to stop a program or not.
 * @args:
 *   void *arg  is the address of the struct program_list which is the node
 *              containing the program_specification linked list & metadata
 **/
static void *routine_master_thrd(void *arg) {
    struct program_list *node = arg;
    struct program_specification *pgm;
    s_client_handler *handler = calloc(CLIENT_MAX_EVENT, sizeof(*handler));
    uint8_t client_event;

    if (!handler)
        exit_thrd(NULL, -1, "calloc() failed", __FILE__, __func__, __LINE__);
    init_handlers(handler);
    launch_all(node);
    while (1) {
        pgm = node->program_linked_list;
        while (pgm) {
            /* if (!PGM_SPEC_GET(nb_thread_alive)) PGM_STATE_SET(started,
             * FALSE); */
            client_event = (PGM_STATE_GET(need_to_restart) * CLIENT_RESTART) +
                           (PGM_STATE_GET(need_to_stop) * CLIENT_STOP) +
                           (PGM_STATE_GET(need_to_start) * CLIENT_START);
            if (client_event) handle_event(handler, pgm, node, client_event);
            pgm = pgm->next;
        }
        usleep(CLIENT_LISTENING_RATE);
    }
    /* TODO: free linked list*/
    free(handler);
    return NULL;
}

/*
 * Set rank id and restart counter in each thread_data struct
 **/
static void init_thread(const struct program_specification *pgm) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(rid, id);
        THRD_DATA_SET(restart_counter, pgm->start_retries);
    }
}

/*
 * If config says that process logs somewhere, init_fd_log() open and
 * store these files
 **/
static uint8_t init_fd_log(struct program_specification *pgm) {
    if (pgm->str_stdout) {
        pgm->log.out = open(pgm->str_stdout, O_RDWR | O_CREAT | O_APPEND, 0755);
        if (pgm->log.out == FD_ERR)
            log_error("open str_stdout failed", __FILE__, __func__, __LINE__);
    }
    if (pgm->str_stderr) {
        pgm->log.err = open(pgm->str_stderr, O_RDWR | O_CREAT | O_APPEND, 0755);
        if (pgm->log.err == FD_ERR)
            log_error("open str_stderr failed", __FILE__, __func__, __LINE__);
    }
    return EXIT_SUCCESS;
}

/*
 * Loop thru struct program_specification linked list to malloc
 * 'number_of_process' struct thread_data and initialize it. Get fd from log
 * files if required
 * @args:
 *   struct program_list *pgm_node  address of a node which contains
 *                                  program_specification linked list & metadata
 **/
static uint8_t init_pgm_spec_list(struct program_list *pgm_node) {
    struct program_specification *pgm = pgm_node->program_linked_list;

    for (uint32_t i = 0; i < pgm_node->number_of_program && pgm; i++) {
        if (init_fd_log(pgm)) return EXIT_FAILURE;
        pgm->thrd = calloc(pgm->number_of_process, sizeof(struct thread_data));
        if (!pgm->thrd)
            log_error("unable to calloc program->thrd", __FILE__, __func__,
                      __LINE__);
        init_thread(pgm);
        pgm->argv = ft_split((char *)pgm->str_start_command, ' ');
        if (!pgm->argv)
            log_error("unable to split program->str_start_command", __FILE__,
                      __func__, __LINE__);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
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
    if (init_pgm_spec_list(node)) return EXIT_FAILURE;
    if (pthread_create(&node->master_thread, &node->attr, routine_master_thrd,
                       node))
        log_error("failed to create master thread", __FILE__, __func__,
                  __LINE__);
    TM_LOG("taskmaster", "program started", NULL);
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
