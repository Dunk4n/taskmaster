#include "taskmaster.h"
#include "minishell.h"

void    print_command_output(struct taskmaster *taskmaster, uint8_t *buffer)
    {
    if(taskmaster == NULL)
        return;
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return;
    if(buffer == NULL)
        return;

    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        {
        if(send_text(taskmaster, (char *) buffer) != EXIT_SUCCESS)
            return;
        }
    else
        {
        write(STDIN_FILENO, buffer, strlen((char *) buffer));
        }

    return;
    }

/**
* WARNING program_linked_list must be lock with mutex_program_linked_list before entering this function
*/
void    display_status_of_program(struct taskmaster *taskmaster, struct program_specification *program)
    {
    if(program == NULL)
        return;
    if(program->global_status.global_status_struct_init == FALSE)
        return;
    if(program->str_name == NULL)
        return;

    //uint32_t cnt;
    uint32_t number_of_proccess_alive;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;
    //cnt = 0;
    number_of_proccess_alive = 0;

    if(program->program_state.started == TRUE)
        {
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is started"COLOR_RESET":\n", program->str_name);
        print_command_output(taskmaster, buffer);
        }
    else
        {
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is NOT started"COLOR_RESET":\n", program->str_name);
        print_command_output(taskmaster, buffer);
        }

    if(program->program_state.need_to_be_removed == TRUE || program->global_status.global_status_configuration_reloading == TRUE)
        {
        if(program->program_state.need_to_be_removed == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Set to be removed\n");
            print_command_output(taskmaster, buffer);
            }
        if(program->global_status.global_status_configuration_reloading == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Configuration is reloading\n");
            print_command_output(taskmaster, buffer);
            }
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "\n");
        print_command_output(taskmaster, buffer);
        }

    if(program->program_state.starting == TRUE || program->program_state.stopping == TRUE || program->program_state.restarting == TRUE)
        {
        if(program->program_state.starting == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Is starting\n");
            print_command_output(taskmaster, buffer);
            }
        if(program->program_state.stopping == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Is stoping\n");
            print_command_output(taskmaster, buffer);
            }
        if(program->program_state.restarting == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Is restarting\n");
            print_command_output(taskmaster, buffer);
            }
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "\n");
        print_command_output(taskmaster, buffer);
        }

    if(program->program_state.need_to_restart == TRUE || program->program_state.need_to_stop == TRUE || program->program_state.need_to_start == TRUE)
        {
        if(program->program_state.need_to_restart == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Set to restart\n");
            print_command_output(taskmaster, buffer);
            }
        if(program->program_state.need_to_stop == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Set to stop\n");
            print_command_output(taskmaster, buffer);
            }
        if(program->program_state.need_to_start == TRUE)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Set to start\n");
            print_command_output(taskmaster, buffer);
            }
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "\n");
        print_command_output(taskmaster, buffer);
        }

    number_of_proccess_alive = 0;
    //TODO if(program->thrd != NULL)
    //    {
    //    cnt = 0;
    //    while(cnt < program->number_of_process)
    //        {
    //        if(program->thrd[cnt].pid != 0)
    //            number_of_proccess_alive++;
    //        cnt++;
    //        }
    //    }

    snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    Number of job alive (%u/%u)\n", number_of_proccess_alive, program->number_of_process);
    print_command_output(taskmaster, buffer);
    }

uint8_t shell_command_status_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->programs.global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    uint8_t cnt;
    uint8_t argument_number;
    struct program_specification *actual_program;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;
    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
            return (EXIT_FAILURE);

        actual_program = taskmaster->programs.program_linked_list;
        while(actual_program != NULL)
            {
            if(actual_program->global_status.global_status_struct_init == FALSE)
                {
                actual_program = NULL;
                break;
                }
            if(actual_program->str_name != NULL && actual_program->name_length > 0)
                {
                snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "    [%s]\n", actual_program->str_name);
                print_command_output(taskmaster, buffer);
                }

            actual_program = actual_program->next;
            }

        if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
            return (EXIT_FAILURE);
        return (EXIT_SUCCESS);
        }

    if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        actual_program = taskmaster->programs.program_linked_list;
        while(actual_program != NULL)
            {
            if(actual_program->global_status.global_status_struct_init == FALSE)
                {
                actual_program = NULL;
                break;
                }
            if(actual_program->str_name != NULL && actual_program->name_length > 0)
                {
                if(strcmp((char *) arguments[cnt], (char *) actual_program->str_name) == 0)
                    {
                    //TODO display status of actual_program
                    //snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Status of program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
                    //print_command_output(taskmaster, buffer);

                    display_status_of_program(taskmaster, actual_program);
                    display_program_specification(actual_program);

                    snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "\n");
                    print_command_output(taskmaster, buffer);
                    break;
                    }
                }

            actual_program = actual_program->next;
            }

        if(actual_program == NULL)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
            print_command_output(taskmaster, buffer);
            }
        cnt++;
        }

    if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

uint8_t shell_command_start_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    struct program_specification *actual_program;
    uint8_t                       argument_number;
    uint8_t                       cnt;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;
    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help

        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Missing program name argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        actual_program = taskmaster->programs.program_linked_list;
        while(actual_program != NULL)
            {
            if(actual_program->global_status.global_status_struct_init == FALSE)
                {
                actual_program = NULL;
                break;
                }
            if(actual_program->str_name != NULL && actual_program->name_length > 0)
                {
                if(strcmp((char *) arguments[cnt], (char *) actual_program->str_name) == 0)
                    {
                    if(actual_program->global_status.global_status_conf_loaded == TRUE)
                        {
                        if(actual_program->program_state.started == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already started"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_be_removed == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_restart == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to restart"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_stop == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to stop"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.starting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.stopping == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.restarting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else
                            {
                            //TODO add in file log start of arguments[cnt]

                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Start program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);

                            actual_program->program_state.need_to_start = TRUE;
                            }
                        break;
                        }
                    break;
                    }
                }

            actual_program = actual_program->next;
            }

        if(actual_program == NULL)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
            print_command_output(taskmaster, buffer);
            }
        cnt++;
        }

    if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

uint8_t shell_command_stop_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    struct program_specification *actual_program;
    uint8_t                       cnt;
    uint8_t                       argument_number;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;
    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Missing program name argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        actual_program = taskmaster->programs.program_linked_list;
        while(actual_program != NULL)
            {
            if(actual_program->global_status.global_status_struct_init == FALSE)
                {
                actual_program = NULL;
                break;
                }
            if(actual_program->str_name != NULL && actual_program->name_length > 0)
                {
                if(strcmp((char *) arguments[cnt], (char *) actual_program->str_name) == 0)
                    {
                    if(actual_program->global_status.global_status_conf_loaded == TRUE)
                        {
                        if(actual_program->program_state.started == FALSE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoped"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_be_removed == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_restart == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to restart"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_start == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to start"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.starting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.stopping == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.restarting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else
                            {
                            //TODO add in file log stop of arguments[cnt]

                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Stop program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);

                            actual_program->program_state.need_to_stop = TRUE;
                            }
                        break;
                        }
                    break;
                    }
                }

            actual_program = actual_program->next;
            }

        if(actual_program == NULL)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
            print_command_output(taskmaster, buffer);
            }
        cnt++;
        }

    if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

uint8_t shell_command_restart_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    struct program_specification *actual_program;
    uint8_t                       cnt;
    uint8_t                       argument_number;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;
    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Missing program name argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        actual_program = taskmaster->programs.program_linked_list;
        while(actual_program != NULL)
            {
            if(actual_program->global_status.global_status_struct_init == FALSE)
                {
                actual_program = NULL;
                break;
                }
            if(actual_program->str_name != NULL && actual_program->name_length > 0)
                {
                if(strcmp((char *) arguments[cnt], (char *) actual_program->str_name) == 0)
                    {
                    if(actual_program->global_status.global_status_conf_loaded == TRUE)
                        {
                        if(actual_program->program_state.need_to_be_removed == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_stop == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to stop"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.need_to_start == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] need to start"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.starting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.stopping == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->program_state.restarting == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            {
                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);
                            }
                        else
                            {
                            //TODO add in file log restart of arguments[cnt]

                            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Restart program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
                            print_command_output(taskmaster, buffer);

                            actual_program->program_state.need_to_restart = TRUE;
                            }
                        break;
                        }
                    break;
                    }
                }

            actual_program = actual_program->next;
            }

        if(actual_program == NULL)
            {
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
            print_command_output(taskmaster, buffer);
            }
        cnt++;
        }

    if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

uint8_t shell_command_reload_conf_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    uint8_t cnt;
    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    if(cnt < 2)
        {
        //TODO missing one argument
        //TODO echo command help
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Missing configuration file argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    if(cnt > 2)
        {
        //TODO too many argument
        //TODO echo command help
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Too many argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "RELOAD_CONF\n");
    print_command_output(taskmaster, buffer);

    //TODO add in file log reloading config file

    free(taskmaster->config_file_path);
    taskmaster->config_file_path = NULL;
    taskmaster->config_file_path = (uint8_t *) strdup((char *) arguments[1]);
    if(taskmaster->config_file_path == NULL)
        {
        log_error("The copying of the configuration file failed", __FILE__, __func__, __LINE__);
        return (EXIT_FAILURE);
        }
    if(reload_config_file(arguments[1], &(taskmaster->programs)) != EXIT_SUCCESS)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" in function "BWHT"%s"CRESET" at line "BWHT"%d"CRESET"\n    The reload of the configuration file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" at line "BWHT"%s"CRESET"\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, ""BRED"ERROR"CRESET"\n");
        #endif

        return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }

uint8_t shell_command_exit_function(struct taskmaster *taskmaster, uint8_t **arguments)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(arguments[0] == NULL)
        return (EXIT_FAILURE);

    uint8_t buffer[OUTPUT_BUFFER_SIZE];

    buffer[0] = NIL;

    if(arguments[1] != NULL)
        {
        //TODO too many argument
        //TODO echo command help
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Too many argument\n");
        print_command_output(taskmaster, buffer);
        return (EXIT_FAILURE);
        }

    stop_and_wait_all_the_program(taskmaster);

    taskmaster->global_status.global_status_exit = TRUE;

    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        {
        if(send_text(taskmaster, EXIT_CLIENT_MARKER) != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }
