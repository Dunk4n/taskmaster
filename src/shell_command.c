#include "taskmaster.h"
#include "minishell.h"

uint8_t  shell_command_status_function(struct taskmaster *taskmaster, uint8_t **arguments)
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
                    ft_printf(BOLD"Status of program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);
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

uint8_t  shell_command_start_function(struct taskmaster *taskmaster, uint8_t **arguments)
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
    uint8_t argument_number;
    struct program_specification *actual_program;

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
                    //TODO check if program can be started ex: need_to_stop
                    if(actual_program->global_status.global_status_conf_loaded == TRUE && actual_program->global_status.global_status_configuration_reloading == FALSE)
                        {
                        if(actual_program->program_state.started == FALSE)
                            {
                            //TODO add in file log start of arguments[cnt]
                            //TODO start actual_program
                            ft_printf(BOLD"Start program ["COLOR_RESET"%s"BOLD"]"COLOR_RESET":\n\n", arguments[cnt]);

                            actual_program->program_state.need_to_start = TRUE;

                            break;
                            }
                        else
                            {
                            ft_printf(BOLD"Program ["COLOR_RESET"%s"BOLD"] is already started"COLOR_RESET":\n\n", arguments[cnt]);

                            break;
                            }
                        }
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

uint8_t  shell_command_stop_function(struct taskmaster *taskmaster, uint8_t **arguments)
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
    uint8_t argument_number;

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

    ft_printf("STOP: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        //TODO add in file log stop of arguments[cnt]
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

    return (EXIT_SUCCESS);
    }

uint8_t  shell_command_restart_function(struct taskmaster *taskmaster, uint8_t **arguments)
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
    uint8_t argument_number;

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

    ft_printf("RESTART: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        //TODO add in file log restart of arguments[cnt]
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

    return (EXIT_SUCCESS);
    }

uint8_t  shell_command_reload_conf_function(struct taskmaster *taskmaster, uint8_t **arguments)
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

uint8_t  shell_command_exit_function(struct taskmaster *taskmaster, uint8_t **arguments)
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

    if(cnt != 1)
        {
        //TODO too many argument
        //TODO echo command help
        ft_printf("Too many argument\n");
        return (EXIT_FAILURE);
        }

    ft_printf("EXIT: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

    //TODO set all program to stop then destroy
    //TODO wait for all the program to stop
    //TODO during wait display status of all the program or number of program still up?
    taskmaster->global_status.global_status_exit = TRUE;

    return (EXIT_SUCCESS);
    }
