#include "taskmaster.h"

static uint8_t is_important_value_changed(struct program_specification *first_program, struct program_specification *second_program)
    {
    if(first_program == NULL || second_program == NULL)
        return (FALSE);

    uint32_t cnt;

    cnt = 0;

    if(strcmp((char *) first_program->str_start_command, (char *) second_program->str_start_command) != 0)
        return (TRUE);

    if(first_program->number_of_process != second_program->number_of_process)
        return (TRUE);

    if(first_program->exit_codes_number != second_program->exit_codes_number)
        return (TRUE);

    cnt = 0;
    while(cnt < first_program->exit_codes_number)
        {
        if(first_program->exit_codes[cnt] != second_program->exit_codes[cnt])
            return (TRUE);

        cnt++;
        }

    if(first_program->stop_signal != second_program->stop_signal)
        return (TRUE);

    if(strcmp((char *) first_program->str_stdout, (char *) second_program->str_stdout) != 0)
        return (TRUE);

    if(strcmp((char *) first_program->str_stderr, (char *) second_program->str_stderr) != 0)
        return (TRUE);

    if(first_program->env_length != second_program->env_length)
        return (TRUE);

    cnt = 0;
    while(cnt < first_program->env_length)
        {
        if(strcmp((char *) first_program->env[cnt], (char *) second_program->env[cnt]) != 0)
            return (TRUE);

        cnt++;
        }

    if(strcmp((char *) first_program->working_dir, (char *) second_program->working_dir) != 0)
        return (TRUE);

    if(first_program->umask != second_program->umask)
        return (TRUE);

    return (FALSE);
    }

/**
* This function reload the YAML config file and update the actual program_list
*/
uint8_t reload_config_file(uint8_t *file_name, struct program_list *program_list)
    {
    if(file_name == NULL)
        return (EXIT_FAILURE);

    if(program_list == NULL)
        return (EXIT_FAILURE);

    if(program_list->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);

    struct program_list           new_program_list;
    struct program_specification *actual_program;
    struct program_specification *actual_program_original_list;
    struct program_specification *previous_program;
    uint32_t                      cnt;
    uint32_t                      cnt_new_list;
    uint32_t                      list_length;

    actual_program               = NULL;
    actual_program_original_list = NULL;
    cnt                          = 0;
    cnt_new_list                 = 0;
    list_length                  = 0;
    previous_program             = NULL;

    new_program_list.global_status.global_status_struct_init = FALSE;
    if(init_program_list(&new_program_list) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(parse_config_file(file_name, &new_program_list) != EXIT_SUCCESS)
        {
        free_program_list(&new_program_list);
        return (EXIT_FAILURE);
        }

    if(pthread_mutex_lock(&program_list->mutex_program_linked_list) != 0)
        return (EXIT_FAILURE);

    if(new_program_list.program_linked_list == NULL)
        {
        free_linked_list_in_program_list(program_list);
        program_list->global_status.global_status_conf_loaded = TRUE;
        }
    else if(program_list->program_linked_list == NULL)
        {
        program_list->program_linked_list = new_program_list.program_linked_list;
        new_program_list.program_linked_list = NULL;

        program_list->last_program_linked_list = new_program_list.last_program_linked_list;
        new_program_list.last_program_linked_list = NULL;

        program_list->number_of_program = new_program_list.number_of_program;
        new_program_list.number_of_program = 0;

        program_list->global_status.global_status_conf_loaded = TRUE;
        }
    else
        {
        actual_program_original_list = program_list->program_linked_list;
        cnt = 0;
        while(cnt < program_list->number_of_program && actual_program_original_list != NULL)
            {
            if(actual_program_original_list->global_status.global_status_struct_init == FALSE)
                {
                if(pthread_mutex_unlock(&program_list->mutex_program_linked_list) != 0)
                    {
                    free_program_list(&new_program_list);
                    return (EXIT_FAILURE);
                    }
                free_program_list(&new_program_list);
                return (EXIT_FAILURE);
                }

            list_length      = new_program_list.number_of_program;
            actual_program   = new_program_list.program_linked_list;
            previous_program = NULL;
            cnt_new_list     = 0;
            while(cnt_new_list < new_program_list.number_of_program && actual_program != NULL)
                {
                if(actual_program->global_status.global_status_struct_init == FALSE)
                    {
                    if(pthread_mutex_unlock(&program_list->mutex_program_linked_list) != 0)
                        {
                        free_program_list(&new_program_list);
                        return (EXIT_FAILURE);
                        }
                    free_program_list(&new_program_list);
                    return (EXIT_FAILURE);
                    }

                if(actual_program_original_list->name_length == actual_program->name_length && strcmp((char *) actual_program_original_list->str_name, (char *) actual_program->str_name) == 0)
                    {
                    if(actual_program_original_list->restart_tmp_program != NULL)
                        {
                        free_program_specification(actual_program_original_list->restart_tmp_program);
                        free(actual_program_original_list->restart_tmp_program);
                        actual_program_original_list->restart_tmp_program = NULL;

                        actual_program_original_list->global_status.global_status_configuration_reloading = FALSE;
                        pthread_mutex_lock(&actual_program_original_list->mtx_pgm_state);
                        actual_program_original_list->program_state.need_to_restart = FALSE;
                        pthread_mutex_unlock(&actual_program_original_list->mtx_pgm_state);
                        }

                    if(is_important_value_changed(actual_program_original_list, actual_program) == TRUE)
                        {
                        pthread_mutex_lock(&actual_program_original_list->mtx_pgm_state);
                        actual_program_original_list->global_status.global_status_configuration_reloading = TRUE;
                        actual_program_original_list->program_state.need_to_restart = TRUE;
                        pthread_mutex_unlock(&actual_program_original_list->mtx_pgm_state);
                        actual_program_original_list->restart_tmp_program = actual_program;

                        actual_program = actual_program->next;

                        if(actual_program_original_list->restart_tmp_program == new_program_list.program_linked_list || previous_program == NULL)
                            new_program_list.program_linked_list = actual_program;
                        else
                            previous_program->next = actual_program;

                        actual_program_original_list->restart_tmp_program->next = NULL;
                        }
                    else
                        {
                        actual_program_original_list->auto_start = actual_program->auto_start;
                        actual_program_original_list->e_auto_restart = actual_program->e_auto_restart;
                        actual_program_original_list->start_time = actual_program->start_time;
                        actual_program_original_list->start_retries = actual_program->start_retries;
                        actual_program_original_list->stop_time = actual_program->stop_time;

                        if(actual_program == new_program_list.program_linked_list || previous_program == NULL)
                            new_program_list.program_linked_list = actual_program->next;
                        else
                            previous_program->next = actual_program->next;

                        free_program_specification(actual_program);
                        free(actual_program);
                        actual_program = NULL;
                        }

                    new_program_list.number_of_program--;
                    break;
                    }

                previous_program = actual_program;
                actual_program = actual_program->next;
                cnt_new_list++;
                }

            if(cnt_new_list >= list_length) {
                pthread_mutex_lock(&actual_program_original_list->mtx_pgm_state);
                actual_program_original_list->program_state.need_to_be_removed = TRUE;
                pthread_mutex_unlock(&actual_program_original_list->mtx_pgm_state);
            }

            actual_program_original_list = actual_program_original_list->next;
            cnt++;
            }

        if(new_program_list.number_of_program > 0)
            {
            if(program_list->last_program_linked_list == NULL)
                {
                if(pthread_mutex_unlock(&program_list->mutex_program_linked_list) != 0)
                    {
                    free_program_list(&new_program_list);
                    return (EXIT_FAILURE);
                    }
                free_program_list(&new_program_list);
                return (EXIT_FAILURE);
                }

            program_list->last_program_linked_list->next = new_program_list.program_linked_list;

            if(new_program_list.last_program_linked_list != NULL)
                program_list->last_program_linked_list = new_program_list.last_program_linked_list;

            program_list->number_of_program += new_program_list.number_of_program;

            new_program_list.program_linked_list = NULL;
            new_program_list.last_program_linked_list = NULL;
            new_program_list.number_of_program = 0;
            }
        }

    if(pthread_mutex_unlock(&program_list->mutex_program_linked_list) != 0)
        return (EXIT_FAILURE);

    free_program_list(&new_program_list);

    return (EXIT_SUCCESS);
    }
