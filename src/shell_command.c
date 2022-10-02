#include "taskmaster.h"
#include "minishell.h"

uint8_t  shell_command_status_function(struct taskmaster *taskmaster, uint8_t **arguments)
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

    ft_printf("STATUS: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

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

    ft_printf("START: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

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

    ft_printf("RELOAD_CONF: ");
    cnt = 1;
    while(arguments[cnt] != NULL && cnt < SHELL_MAX_ARGUMENT)
        {
        ft_printf(" [%s]", arguments[cnt]);
        cnt++;
        }
    ft_printf("\n");

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
