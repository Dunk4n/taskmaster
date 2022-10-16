#include "tm_job_control.h"

#include "taskmaster.h"

/* getters */

/* enum pgm_states { */
/*     started = 1 << 0, */
/*     need_to_restart = 1 << 1, */
/*     restarting = 1 << 2, */
/*     need_to_stop = 1 << 3, */
/*     stopping = 1 << 4, */
/*     need_to_start = 1 << 5, */
/*     starting = 1 << 6, */
/*     need_to_be_removed = 1 << 7 */
/* }; */

/* bool pgm_state_getter(struct program_specification *pgm, */
/*                       enum pgm_states state) { */
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
static void exit_thread(struct program_specification *pgm, int32_t id) {
    if (!pgm || id == -1) return;
    THRD_DATA_SET(pid, 0);
    THRD_DATA_SET(tid, 0);
    pgm->nb_thread_alive -= 1;
    if (PGM_SPEC_GET(nb_thread_alive) <= 0) PGM_STATE_SET(started, FALSE);
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
    if (pgm->umask) umask(pgm->umask); /* default file mode creation */
    /* privilege de-escalation if launched in sudo mode */
    if (getuid() == 0)
        if (setuid(atoi(getenv("SUDO_UID"))) == -1)
            TM_LOG("setuid()", "pid [%d] - failed to set uid to %d", getpid(),
                   atoi(getenv("SUDO_UID")));
    if (pgm->working_dir) {
        if (chdir((char *)pgm->working_dir) == -1)
            TM_LOG("chdir()", "pid [%d] - failed to change directory to %s",
                   getpid(), PGM_SPEC_GET(working_dir));
    }
    if (pgm->log.out != UNINITIALIZED_FD) dup2(pgm->log.out, STDOUT_FILENO);
    if (pgm->log.err != UNINITIALIZED_FD) dup2(pgm->log.err, STDERR_FILENO);
    if (execve(pgm->argv[0], pgm->argv, (char **)pgm->env) == -1) {
        TM_LOG("execve()", "pid [%d] - failed to call processus %s", getpid(),
               PGM_SPEC_GET(str_name));
        err_display("execve() failed", __FILE__, __func__, __LINE__);
    }
    // BUG. Sur un cas ou on fait retry 8 fois un pgm avec 1 proc, qui n'existe
    // pas et qui donc fail execve(), il y a parfois un bug: parfois execve ne
    // fail pas et le programme en question qui est lancé - et visible avec
    // la commande 'ps -aux | grep taskmaster' est... taskmaster avec le même
    // fichier de config en arguments.
}

static void *routine_start_supervisor(void *arg) {
    s_time_control *time_control = arg;
    struct program_specification *pgm = time_control->pgm;
    struct program_list *node = pgm->node;
    uint32_t id = time_control->rid;

    while (timediff(&time_control->start) < PGM_SPEC_GET(start_time)) {
        if (time_control->pid != THRD_DATA_GET(pid) || time_control->exit) {
            TM_START_LOG("DIDN'T LAUNCHED CORRECTLY");
            return NULL;
        }
        usleep(START_SUPERVISOR_RATE);
    }
    TM_START_LOG("LAUNCHED CORRECTLY");

    return NULL;
}

/*
 * Update information of the thread_data struct related to one process - the
 * timestamp, pid & restart_counter.
 * Launch an attached timer thread which monitor if the process is still alive
 * after 'start_time' seconds.
 **/
static void thread_data_update(struct program_list *node,
                               struct program_specification *pgm, int32_t id,
                               pid_t pid, s_time_control *time_control) {
    struct timeval start;

    gettimeofday(&start, NULL);
    THRD_DATA_SET(start_timestamp, start);
    THRD_DATA_SET(pid, pid);
    THRD_DATA_SET(restart_counter, THRD_DATA_GET(restart_counter) - 1);
    TM_THRD_LOG("LAUNCHED");
    debug_thrd();
    time_control[THRD_DATA_GET(restart_counter)].pid = pid;
    time_control[THRD_DATA_GET(restart_counter)].start = start;
    if (pthread_create(&time_control[THRD_DATA_GET(restart_counter)].thrd_id,
                       NULL, routine_start_supervisor,
                       &time_control[THRD_DATA_GET(restart_counter)]))
        err_display("failed to create start supervisor thread", __FILE__,
                    __func__, __LINE__);
}

static void init_time_control(struct program_specification *pgm, int32_t id,
                              s_time_control *time_control) {
    for (uint32_t i = 0; i < PGM_SPEC_GET(start_retries) + 1; i++) {
        time_control[i].pgm = pgm;
        time_control[i].rid = id;
    }
}

static void *exit_launcher_thread(struct program_list *node,
                                  struct program_specification *pgm,
                                  s_time_control *time_control, int32_t id) {
    TM_THRD_LOG("EXITED");
    exit_thread(pgm, id);
    for (uint32_t i = 0; i < PGM_SPEC_GET(start_retries) + 1; i++)
        time_control[i].exit = TRUE;
    for (uint32_t i = 0; i < PGM_SPEC_GET(start_retries) + 1; i++)
        pthread_join(time_control[i].thrd_id, NULL);
    free(time_control);
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
    struct program_specification *pgm = arg;
    struct program_list *node = pgm->node;
    s_time_control *time_control;
    const int32_t id = get_id(pgm);
    int32_t pgm_restart = 1;
    pid_t pid;

    pgm->nb_thread_alive += 1;
    if (id == -1)
        exit_thrd(NULL, id, "couldn't find thread id", __FILE__, __func__,
                  __LINE__);
    time_control =
        calloc(PGM_SPEC_GET(start_retries) + 1, sizeof(*time_control));
    if (!time_control)
        exit_thrd(pgm, id, "calloc() failed", __FILE__, __func__, __LINE__);
    init_time_control(pgm, id, time_control);

    while (pgm_restart > 0) {
        /* the more it restarts the more it sleeps (supervisord behavior) */
        sleep((PGM_SPEC_GET(start_retries) + 1) -
              THRD_DATA_GET(restart_counter));

        pid = fork();
        if (pid == -1)
            exit_thrd(pgm, id, "fork() failed", __FILE__, __func__, __LINE__);
        if (pid == 0) {
            configure_and_launch(node, pgm);
            exit(EXIT_FAILURE);
        } else {
            thread_data_update(node, pgm, id, pid, time_control);
            child_control(pgm, id, pid);
            pgm_restart =
                pgm->e_auto_restart * (THRD_DATA_GET(restart_counter));
        }
    }

    return exit_launcher_thread(node, pgm, time_control, id);
}

/*
 * Creates all launcher_threads from one struct program_specification, in a
 * detached mode
 * If the thread already exists the restart_counter is still reset.
 **/
static uint8_t create_launcher_threads(struct program_list *node,
                                       struct program_specification *pgm) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(restart_counter, PGM_SPEC_GET(start_retries) + 1);
        if (THRD_DATA_GET(tid) == 0 && THRD_DATA_GET(pid) == 0) {
            if (pthread_create(&pgm->thrd[id].tid, &node->attr,
                               routine_launcher_thrd, pgm))
                log_error("failed to create launcher thread", __FILE__,
                          __func__, __LINE__);
        }
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
        THRD_DATA_SET(restart_counter, 0);
        if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) {
            kill(THRD_DATA_GET(pid), pgm->stop_signal);
        }
    }
    return EXIT_SUCCESS;
}

/*
 * Kills all processus from one struct program_specification with SIGTERM
 * Does nothing if the thread is already down.
 **/
static uint8_t kill_all_processus(struct program_specification *pgm) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        THRD_DATA_SET(restart_counter, 0);
        if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) {
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

/*
 * starts threads which aren't already started. Set program state.
 **/
static uint8_t do_start(struct program_specification *pgm,
                        struct program_list *node) {
    struct stat statbuf;

    pthread_mutex_lock(&pgm->mtx_client_event);

    if (PGM_STATE_GET(starting) || PGM_STATE_GET(stopping)) {
        pthread_mutex_unlock(&pgm->mtx_client_event);
        return EXIT_FAILURE;
    }
    PGM_STATE_SET(need_to_start, FALSE);
    PGM_STATE_SET(starting, TRUE);
    TM_LOG("start", "%s", PGM_SPEC_GET(str_name));

    pthread_mutex_unlock(&pgm->mtx_client_event);

    if (stat(PGM_SPEC_GET(argv)[0], &statbuf) == -1)
        TM_LOG("start error", "%s can't be executed", PGM_SPEC_GET(argv)[0]);
    else {
        create_launcher_threads(node, pgm);
        PGM_STATE_SET(started, TRUE);
    }

    PGM_STATE_SET(starting, FALSE);
    return EXIT_SUCCESS;
}

/* checks if the processes are stopped in the given time */
void *routine_stop_supervisor(void *arg) {
    struct program_specification *pgm = arg;
    struct program_list *node = pgm->node;
    uint8_t alive = TRUE;

    while (alive &&
           timediff(&PGM_SPEC_GET(stop_timestamp)) < PGM_SPEC_GET(stop_time)) {
        usleep(STOP_SUPERVISOR_RATE);
        alive = FALSE;
        for (uint32_t id = 0; id < PGM_SPEC_GET(number_of_process); id++)
            if (THRD_DATA_GET(tid) && THRD_DATA_GET(pid)) alive = TRUE;
    }
    if (alive) {
        kill_all_processus(pgm);
        TM_STOP_LOG("PROCESSUS HAD BEEN KILLED");
    } else
        TM_STOP_LOG("PROCESSUS STOPPED AS EXPECTED");

    PGM_STATE_SET(stopping, FALSE);
    PGM_STATE_SET(started, FALSE);
    return NULL;
}

/*
 * Stop threads which are alive. Update program state. Check if processus
 * stopped quickly enough according to config file and kill them if not.
 **/
static uint8_t do_stop(struct program_specification *pgm,
                       struct program_list *node) {
    pthread_t tid;

    pthread_mutex_lock(&pgm->mtx_client_event);
    if (PGM_STATE_GET(starting) || PGM_STATE_GET(stopping)) {
        pthread_mutex_unlock(&pgm->mtx_client_event);
        return EXIT_FAILURE;
    }

    PGM_STATE_SET(stopping, TRUE);
    PGM_STATE_SET(need_to_stop, FALSE);
    TM_LOG("stop", "%s", PGM_SPEC_GET(str_name));
    pthread_mutex_unlock(&pgm->mtx_client_event);
    stop_all_processus(pgm, node);

    if (pthread_create(&tid, &node->attr, routine_stop_supervisor, pgm)) {
        PGM_STATE_SET(stopping, FALSE);
        PGM_STATE_SET(started, FALSE);
        TM_LOG("stop control",
               "[%s] - failed to create routine_timer_stop_watch thread",
               PGM_SPEC_GET(str_name));
        log_error("failed to create routine_timer_stop_watch thread", __FILE__,
                  __func__, __LINE__);
    }

    return EXIT_SUCCESS;
}

static uint8_t do_restart(struct program_specification *pgm,
                          struct program_list *node) {
    pthread_mutex_lock(&pgm->mtx_client_event);
    if (PGM_STATE_GET(starting) || PGM_STATE_GET(stopping) ||
        PGM_STATE_GET(restarting)) {
        pthread_mutex_unlock(&pgm->mtx_client_event);
        return EXIT_FAILURE;
    }

    PGM_STATE_SET(need_to_restart, FALSE);
    PGM_STATE_SET(restarting, TRUE);
    TM_LOG("restart", "%s", PGM_SPEC_GET(str_name));
    pthread_mutex_unlock(&pgm->mtx_client_event);
    if (do_stop(pgm, node)) goto exit_restart;
    if (do_start(pgm, node)) goto exit_restart;

exit_restart:
    PGM_STATE_SET(restarting, FALSE);
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
        if (pgm->auto_start) PGM_STATE_SET(need_to_start, TRUE);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
}

/* static void destroy_pgm(struct program_specification *pgm) { */
/*     if (!pgm) return; */
/*     pthread_mutex_destroy(&pgm->mtx_client_event); */
/*     pthread_mutex_destroy(&pgm->mtx_pgm_state); */
/*     if (pgm->str_name) { */
/*         free(pgm->str_name); */
/*         pgm->str_name = NULL; */
/*     } */
/*     if (pgm->str_start_command) { */
/*         free(pgm->str_start_command); */
/*         pgm->str_start_command = NULL; */
/*     } */
/*     if (pgm->exit_codes) { */
/*         free(pgm->exit_codes); */
/*         pgm->exit_codes = NULL; */
/*     } */
/*     if (pgm->str_stdout) { */
/*         free(pgm->str_stdout); */
/*         pgm->str_stdout = NULL; */
/*     } */
/*     if (pgm->str_stderr) { */
/*         free(pgm->str_stderr); */
/*         pgm->str_stderr = NULL; */
/*     } */
/*     if (pgm->working_dir) { */
/*         free(pgm->working_dir); */
/*         pgm->working_dir = NULL; */
/*     } */
/*     if (pgm->thrd) { */
/*         free(pgm->thrd); */
/*         pgm->thrd = NULL; */
/*     } */
/*     if (pgm->env) { */
/*         for (uint32_t i = 0; pgm->env[i]; i++) { */
/*             free(pgm->env[i]); */
/*             pgm->env[i] = NULL; */
/*         } */
/*         free(pgm->env); */
/*         pgm->env = NULL; */
/*     } */
/*     if (pgm->argv) { */
/*         for (uint32_t i = 0; pgm->argv[i]; i++) { */
/*             free(pgm->argv[i]); */
/*             pgm->argv[i] = NULL; */
/*         } */
/*         free(pgm->argv); */
/*         pgm->argv = NULL; */
/*     } */
/* } */

static void exit_job_control(struct program_list *node) {
    struct program_specification *pgm, *tmp;

    TM_LOG("exit", "...", NULL);
    for (pgm = node->program_linked_list; pgm; pgm = pgm->next)
        do_stop(pgm, node);
    for (int i = 1; i;) {
        i = 0;
        for (pgm = node->program_linked_list; pgm; pgm = pgm->next)
            if (pgm->program_state.stopping) i++;
        usleep(EXIT_MASTER_RATE);
    }
    for (pgm = node->program_linked_list; pgm;) {
        tmp = pgm->next;
        /* destroy_pgm(pgm); */
        pgm = tmp;
    }
}

/*
 * The master thread listen the clients events - start, stop, restart - and
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
    while (node->global_status.exit == FALSE) {
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
    TM_LOG("taskmaster", "program exit", NULL);
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
