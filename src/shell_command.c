#include "taskmaster.h"
#include "minishell.h"

/**
* WARNING program_linked_list must be lock with mutex_program_linked_list before entering this function
*/
void    display_status_of_program(struct program_specification *program)
    {
    if(program == NULL)
        return;
    if(program->global_status.global_status_struct_init == FALSE)
        return;
    if(program->str_name == NULL)
        return;

    //uint32_t cnt;
    uint32_t number_of_proccess_alive;

    //cnt = 0;
    number_of_proccess_alive = 0;

    if(program->program_state.started == TRUE)
        ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is started"COLOR_RESET":\n", program->str_name);
    else
        ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is NOT started"COLOR_RESET":\n", program->str_name);

    if(program->program_state.need_to_be_removed == TRUE || program->global_status.global_status_configuration_reloading == TRUE)
        {
        if(program->program_state.need_to_be_removed == TRUE)
            ft_printf("    Set to be removed\n");
        if(program->global_status.global_status_configuration_reloading == TRUE)
            ft_printf("    Configuration is reloading\n");
        ft_printf("\n");
        }

    if(program->program_state.starting == TRUE || program->program_state.stoping == TRUE || program->program_state.restarting == TRUE)
        {
        if(program->program_state.starting == TRUE)
            ft_printf("    Is starting\n");
        if(program->program_state.stoping == TRUE)
            ft_printf("    Is stoping\n");
        if(program->program_state.restarting == TRUE)
            ft_printf("    Is restarting\n");
        ft_printf("\n");
        }

    if(program->program_state.need_to_restart == TRUE || program->program_state.need_to_stop == TRUE || program->program_state.need_to_start == TRUE)
        {
        if(program->program_state.need_to_restart == TRUE)
            ft_printf("    Set to restart\n");
        if(program->program_state.need_to_stop == TRUE)
            ft_printf("    Set to stop\n");
        if(program->program_state.need_to_start == TRUE)
            ft_printf("    Set to start\n");
        ft_printf("\n");
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

    ft_printf("    Number of job alive (%u/%u)\n", number_of_proccess_alive, program->number_of_process);
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
                ft_printf("    [%s]\n", actual_program->str_name);
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
                    //ft_printf(BOLD"Status of program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
                    display_status_of_program(actual_program);
                    display_program_specification(actual_program);
                    ft_printf("\n");
                    break;
                    }
                }

            actual_program = actual_program->next;
            }

        if(actual_program == NULL)
            {
            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
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

    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help
        ft_printf("Missing program name argument\n");
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
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already started"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_be_removed == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_restart == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to restart"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_stop == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to stop"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.starting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.stoping == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.restarting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                        else
                            {
                            //TODO add in file log start of arguments[cnt]

                            ft_printf(BOLD"Start program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);

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
            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
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

    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help
        ft_printf("Missing program name argument\n");
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
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoped"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_be_removed == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_restart == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to restart"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_start == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to start"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.starting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.stoping == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.restarting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                        else
                            {
                            //TODO add in file log stop of arguments[cnt]

                            ft_printf(BOLD"Stop program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);

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
            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
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

    actual_program = NULL;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    argument_number = cnt - 1;

    if(argument_number == 0)
        {
        //TODO missing arguments
        //TODO echo command help
        ft_printf("Missing program name argument\n");
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
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to be removed"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_stop == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to stop"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.need_to_start == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] need to start"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.starting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already starting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.stoping == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already stoping"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->program_state.restarting == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already restarting"COLOR_RESET":\n\n", arguments[cnt]);
                        else if(actual_program->global_status.global_status_configuration_reloading == TRUE)
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] configuration is reloading"COLOR_RESET":\n\n", arguments[cnt]);
                        else
                            {
                            //TODO add in file log restart of arguments[cnt]

                            ft_printf(BOLD"Restart program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);

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
            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] not found\n\n"COLOR_RESET, arguments[cnt]);
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

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        cnt++;

    if(cnt < 2)
        {
        //TODO missing one argument
        //TODO echo command help
        ft_printf("Missing configuration file argument\n");
        return (EXIT_FAILURE);
        }

    if(cnt > 2)
        {
        //TODO too many argument
        //TODO echo command help
        ft_printf("Too many argument\n");
        return (EXIT_FAILURE);
        }

    ft_printf("RELOAD_CONF\n");

    //TODO add in file log reloading config file

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

    if(arguments[1] != NULL)
        {
        //TODO too many argument
        //TODO echo command help
        ft_printf("Too many argument\n");
        return (EXIT_FAILURE);
        }

    stop_and_wait_all_the_program(&(taskmaster->programs));

    taskmaster->global_status.global_status_exit = TRUE;

    return (EXIT_SUCCESS);
    }
