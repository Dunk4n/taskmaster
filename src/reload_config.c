#include "taskmaster.h"
#include "tm_job_control.h"

static void add_event(struct program_list *node, struct s_event event) {
    sem_wait(&node->free_place);
    pthread_mutex_lock(&node->mtx_queue);
    node->event_queue[node->ev_queue_size] = event;
    node->ev_queue_size++;
    pthread_mutex_unlock(&node->mtx_queue);
    sem_post(&node->new_event);
}

static uint8_t pgm_compare(struct program_specification *pgm_oldlst,
                           struct program_specification *pgm_newlst) {
    bool soft_reload =
        (pgm_oldlst->auto_start != pgm_newlst->auto_start ||
         pgm_oldlst->e_auto_restart != pgm_newlst->e_auto_restart ||
         pgm_oldlst->start_time != pgm_newlst->start_time ||
         pgm_oldlst->start_retries != pgm_newlst->start_retries ||
         pgm_oldlst->stop_signal != pgm_newlst->stop_signal ||
         pgm_oldlst->stop_time != pgm_newlst->stop_time);
    bool hard_reload =
        (strcmp((char *)pgm_oldlst->str_start_command,
                (char *)pgm_newlst->str_start_command) ||
         pgm_oldlst->number_of_process != pgm_newlst->number_of_process ||
         pgm_oldlst->exit_codes_number != pgm_newlst->exit_codes_number ||
         strcmp((char *)pgm_oldlst->str_stdout,
                (char *)pgm_newlst->str_stdout) ||
         strcmp((char *)pgm_oldlst->str_stderr,
                (char *)pgm_newlst->str_stderr) ||
         pgm_oldlst->env_length != pgm_newlst->env_length ||
         strcmp((char *)pgm_oldlst->working_dir,
                (char *)pgm_newlst->working_dir) ||
         pgm_oldlst->umask != pgm_newlst->umask);

    for (uint32_t cnt = 0; cnt < pgm_oldlst->env_length && !hard_reload; cnt++)
        if (strcmp((char *)pgm_oldlst->env[cnt],
                   (char *)pgm_newlst->env[cnt]) != 0)
            hard_reload = true;

    for (uint32_t cnt = 0; cnt < pgm_oldlst->exit_codes_number && !hard_reload;
         cnt++)
        if (pgm_oldlst->exit_codes[cnt] != pgm_newlst->exit_codes[cnt])
            soft_reload = true;

    if (hard_reload)
        return CLIENT_HARD_RELOAD;
    else if (soft_reload)
        return CLIENT_SOFT_RELOAD;
    else
        return EXIT_SUCCESS;
}

/* remove pgm from linked list hold in node. prev must be the link before pgm */
static void remove_link(struct program_list *node,
                        struct program_specification *prev,
                        struct program_specification *pgm) {
    if (!prev)
        node->program_linked_list = pgm->next;
    else
        prev->next = pgm->next;
    if (node->last_program_linked_list == pgm)
        node->last_program_linked_list = prev;
}

/*
 * detect differences of matching pgm between two lists.
 * - soft reload: a copy from some new_pgm values to old_pgm will be done.
 * - hard reload: new_pgm will take place of old_pgm in the old list
 */
static void ev_reload(struct program_list *node,
                      struct program_list *new_node) {
    struct program_specification *pgm_oldlst = node->program_linked_list,
                                 *pgm_oldlst_prev = NULL, *pgm_newlst_prev;
    uint8_t ret;

    while (pgm_oldlst) {
        pgm_newlst_prev = NULL;
        for (struct program_specification *pgm_newlst =
                 new_node->program_linked_list;
             pgm_newlst; pgm_newlst = pgm_newlst->next) {
            if (!strcmp((char *)pgm_oldlst->str_name,
                        (char *)pgm_newlst->str_name)) {
                ret = pgm_compare(pgm_oldlst, pgm_newlst);

                if (ret == CLIENT_SOFT_RELOAD) {
                    /* keep new pgm ptr in old pgm to be used by do_reload() */
                    pgm_oldlst->restart_tmp_program = pgm_newlst;
                    /* remove pgm_newlst from the new_list */
                    remove_link(new_node, pgm_newlst_prev, pgm_newlst);

                    add_event(node,
                              (struct s_event){pgm_oldlst, CLIENT_SOFT_RELOAD});
                } else if (ret == CLIENT_HARD_RELOAD) {
                    /* keep new pgm ptr in old pgm to be used by do_reload() */
                    pgm_oldlst->restart_tmp_program = pgm_newlst;
                    /* keep previous old pgm ptr in new pgm for do_reload() */
                    pgm_newlst->restart_tmp_program = pgm_oldlst_prev;
                    /* remove pgm_newlst from the new_list */
                    remove_link(new_node, pgm_newlst_prev, pgm_newlst);

                    /* next iteration of pgm_oldlst will have pgm_newlst as
                    previous pgm as pgm_newlst will take place into old list */
                    pgm_oldlst_prev = pgm_newlst;

                    add_event(node,
                              (struct s_event){pgm_oldlst, CLIENT_HARD_RELOAD});
                }
                break;
            }
            pgm_newlst_prev = pgm_newlst;
        }
        if (ret != CLIENT_HARD_RELOAD) pgm_oldlst_prev = pgm_oldlst;
        pgm_oldlst = pgm_oldlst->next;
    }
}

/* remove from the old list a pgm not found in the new list and add event. */
static void ev_delete(struct program_list *node,
                      struct program_list *new_node) {
    struct program_specification *pgm_oldlst = node->program_linked_list,
                                 *prev = NULL, *tmp;
    bool match;

    while (pgm_oldlst) {
        match = false;

        for (struct program_specification *pgm_newlst =
                 new_node->program_linked_list;
             pgm_newlst; pgm_newlst = pgm_newlst->next) {
            if (!strcmp((char *)pgm_oldlst->str_name,
                        (char *)pgm_newlst->str_name)) {
                match = true;
                break;
            }
        }

        /* no match means the current old pgm doesn't exist in the new list */
        if (!match) {
            /* remove pgm_oldlst from the old_list */
            remove_link(node, prev, pgm_oldlst);

            tmp = pgm_oldlst;
            pgm_oldlst = pgm_oldlst->next;

            add_event(node, (struct s_event){tmp, CLIENT_DEL});
        } else {
            prev = pgm_oldlst;
            pgm_oldlst = pgm_oldlst->next;
        }
    }
}

/*
 * transfer from the new list to the old list a pgm from the new list
 * not found in the old list and add event
 */
static void ev_add(struct program_list *node, struct program_list *new_node) {
    struct program_specification *pgm_newlst = new_node->program_linked_list,
                                 *prev = NULL, *tmp;
    bool match;

    while (pgm_newlst) {
        match = false;

        for (struct program_specification *pgm_oldlst =
                 node->program_linked_list;
             pgm_oldlst; pgm_oldlst = pgm_oldlst->next) {
            if (!strcmp((char *)pgm_newlst->str_name,
                        (char *)pgm_oldlst->str_name)) {
                match = true;
                break;
            }
        }

        /* no match means the current new pgm doesn't exist in the old list */
        if (!match) {
            /* add pgm_newlst to the old_list */
            if (!node->program_linked_list) {
                node->program_linked_list = pgm_newlst;
                node->last_program_linked_list = pgm_newlst;
            } else {
                node->last_program_linked_list->next = pgm_newlst;
                node->last_program_linked_list = pgm_newlst;
            }

            /* remove pgm_newlst from the new_list */
            remove_link(new_node, prev, pgm_newlst);

            tmp = pgm_newlst;
            pgm_newlst = pgm_newlst->next;

            /* update its next pointer */
            tmp->next = NULL;

            add_event(node, (struct s_event){tmp, CLIENT_ADD});
        } else {
            prev = pgm_newlst;
            pgm_newlst = pgm_newlst->next;
        }
    }
}

/*
 * The new pgm from the list have their node variable pointing
 * to the temporary new_node, which won't be valid in any case. This issue is
 * because of the design of parse_config_file(), so we correct it here.
 */
static void prepare_list(struct program_specification *pgm_newlst,
                         struct program_list *node) {
    while (pgm_newlst) {
        pgm_newlst->node = node;
        for (uint32_t i = 0; i < pgm_newlst->number_of_process; i++)
            (&pgm_newlst->thrd[i])->node = node;
        pgm_newlst = pgm_newlst->next;
    }
}

/**
 * destroy struct program_list 'node'. Will destroy only pgm which are identical
 * to old_list. Other pgm are either integrated into old_list or destroyed
 * during process.
 */
static void destroy_node(struct program_list *node) {
    pthread_attr_destroy(&node->attr);
    close(node->tm_fd_log);

    free_linked_list_in_program_list(node);
    sem_destroy(&node->free_place);
    sem_destroy(&node->new_event);
    pthread_mutex_destroy(&(node->mtx_queue));

    pthread_mutex_destroy(&(node->mutex_program_linked_list));
    pthread_mutex_destroy(&(node->mtx_log));
}

/*
 * reload the YAML config file & set according events to reload/delete or add
 * a program.
 */
uint8_t reload_config_file(uint8_t *file_name, struct program_list *node) {
    struct program_list new_node;

    TM_LOG2("reload", "...", NULL);
    if (!file_name || !node ||
        node->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);

    new_node.global_status.global_status_struct_init = FALSE;
    if (init_program_list(&new_node) != EXIT_SUCCESS) return (EXIT_FAILURE);

    if (parse_config_file(file_name, &new_node) != EXIT_SUCCESS) {
        free_program_list(&new_node);
        return (EXIT_FAILURE);
    }

    prepare_list(new_node.program_linked_list, node);

    ev_delete(node, &new_node);
    ev_add(node, &new_node);
    ev_reload(node, &new_node);

    destroy_node(&new_node);

    return EXIT_SUCCESS;
}
