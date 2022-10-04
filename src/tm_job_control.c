#include "taskmaster.h"

#define UPDATE_THRD_DATA(name, value) \
    do {                              \
        pgm->thrd[id].name = value;   \
    } while (0)

#define GET_THRD_DATA(name) pgm->thrd[id].name

void exit_thread(struct program_specification *pgm, int32_t id) {
    if (!pgm || id == -1) return;
    UPDATE_THRD_DATA(pid, 0);
    UPDATE_THRD_DATA(tid, 0);
}

static int32_t get_id(struct program_specification *pgm) {
    pthread_t tid = pthread_self();
    for (uint32_t i = 0; i < pgm->number_of_process; i++) {
        if (tid == pgm->thrd[i].tid && i == pgm->thrd[i].rid) return i;
    }
    return -1;
}

static int32_t child_control(struct program_specification *pgm, int32_t id,
                             pid_t pid) {
    int32_t wstatus, child_ret = 0;

    waitpid(pid, &wstatus, 0);
    if (WIFEXITED(wstatus)) {
        child_ret = WEXITSTATUS(wstatus);
        UPDATE_THRD_DATA(exit_status, child_ret);
        printf("EXIT: exit_status: %d\n", child_ret);
    } else if (WIFSIGNALED(wstatus)) {
        child_ret = WTERMSIG(wstatus);
        printf("SIGNAL: exit_status: %d\n", child_ret);
    }
    UPDATE_THRD_DATA(restart_counter, GET_THRD_DATA(restart_counter) - 1);
    return child_ret;
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
    int32_t id = get_id(pgm), pgm_restart = 1;
    pid_t pid;

    if (id == -1)
        exit_thrd(NULL, id, "couldn't find thread id", __FILE__, __func__,
                  __LINE__);
    debug_thrd();
    while (pgm_restart) {
        printf("restart: %u\n", GET_THRD_DATA(restart_counter));
        pid = fork();
        if (pid == -1)
            exit_thrd(NULL, id, "fork() failed", __FILE__, __func__, __LINE__);
        if (pid == 0) {
            if (pgm->log.out != UNINITIALIZED_FD)
                dup2(pgm->log.out, STDOUT_FILENO);
            if (pgm->log.err != UNINITIALIZED_FD)
                dup2(pgm->log.err, STDERR_FILENO);
            if (execve(pgm->argv[0], pgm->argv, (char **)pgm->env) == -1)
                exit_thrd(NULL, id, "execve() failed", __FILE__, __func__,
                          __LINE__);
            exit(EXIT_FAILURE);
        } else {
            child_control(pgm, id, pid);
            pgm_restart = pgm->e_auto_restart * GET_THRD_DATA(restart_counter);
        }
    }
    return NULL;
}

/*
 * Creates all launcher_threads from one struct program_specification, in a
 * detached mode
 **/
static uint8_t start_launcher_thread(struct program_specification *pgm,
                                     const pthread_attr_t *attr) {
    for (uint32_t i = 0; i < pgm->number_of_process; i++) {
        if (pthread_create(&pgm->thrd[i].tid, attr, routine_launcher_thrd, pgm))
            log_error("failed to create launcher thread", __FILE__, __func__,
                      __LINE__);
    }
    return EXIT_SUCCESS;
}

/*
 * wrapper around start_launcher_thread() to loop thru the pgm_spec linked list
 **/
static uint8_t launch_all(const struct program_list *pm) {
    struct program_specification *pgm = pm->program_linked_list;

    for (uint32_t i = 0; i < pm->number_of_program; i++) {
        start_launcher_thread(pgm, &pm->attr);
        pgm = pgm->next;
    }
    return EXIT_SUCCESS;
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
    launch_all(arg);
    /* Here, code to monitor all that shit */
    while (1) {
        sleep(3);
        printf("master thread watching\n");
    }
    return NULL;
}

/*
 * Set rank id and restart counter in each thread_data struct
 **/
static void init_thread(const struct program_specification *pgm) {
    for (uint32_t id = 0; id < pgm->number_of_process; id++) {
        UPDATE_THRD_DATA(rid, id);
        UPDATE_THRD_DATA(restart_counter, pgm->start_retries);
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
uint8_t tm_job_control(struct program_list *pgm_node) {
    if (init_pgm_spec_list(pgm_node)) return EXIT_FAILURE;
    if (pthread_create(&pgm_node->master_thread, &pgm_node->attr,
                       routine_master_thrd, pgm_node))
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
