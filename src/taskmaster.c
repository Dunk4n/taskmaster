#include "taskmaster.h"
#include "minishell.h"

uint8_t taskmaster_shell(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->command_line.global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);

    char    *line;

    line = NULL;

    while(taskmaster->global_status.global_status_exit == FALSE)
        {
        line = NULL;
        if(!get_edit_line(&(taskmaster->command_line), &line))
            {
            free(line);
            line = NULL;
            return (EXIT_FAILURE);
            }
        if(line == NULL)
            {
            write(STDOUT_FILENO, SHELL_EXIT_MESSAGE, sizeof(SHELL_EXIT_MESSAGE) - 1);
            return (EXIT_SUCCESS);
            }
        if(add_in_history(&(taskmaster->command_line), line) != EXIT_SUCCESS)
            {
            free(line);
            line = NULL;
            return (EXIT_FAILURE);
            }

        if(taskmaster->global_status.global_status_start_as_client == TRUE)
            {
            if(send_command_line_to_daemon(taskmaster, line) != EXIT_SUCCESS)
                {
                free(line);
                line = NULL;
                return (EXIT_FAILURE);
                }
            }
        else
            {
            if(execute_command_line(taskmaster, line) != EXIT_SUCCESS)
                {
                free(line);
                line = NULL;
                return (EXIT_FAILURE);
                }
            }

        free(line);
        line = NULL;
        }

    return (EXIT_SUCCESS);
    }

uint8_t init_taskmaster(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == TRUE)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_start_as_daemon == TRUE && taskmaster->global_status.global_status_start_as_client == TRUE)
        return (EXIT_FAILURE);

    taskmaster->global_status.global_status_exit = FALSE;

    taskmaster->socket = -1;
    taskmaster->client_socket = -1;

    taskmaster->command_line.global_status.global_status_struct_init = FALSE;
    if(taskmaster->global_status.global_status_start_as_daemon == FALSE)
        {
        if(init_command_line(&(taskmaster->command_line)) != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        }

    taskmaster->programs.global_status.global_status_struct_init = FALSE;
    if(taskmaster->global_status.global_status_start_as_client == FALSE)
        {
        if(init_program_list(&(taskmaster->programs)) != EXIT_SUCCESS)
            {
            free_command_line(&(taskmaster->command_line));
            return (EXIT_FAILURE);
            }
        }

    if(taskmaster->global_status.global_status_start_as_daemon == TRUE)
        {
        if(init_taskmaster_daemon(taskmaster) != EXIT_SUCCESS)
            {
            free_program_list(&(taskmaster->programs));
            free_command_line(&(taskmaster->command_line));
            return (EXIT_FAILURE);
            }
        }
    else if(taskmaster->global_status.global_status_start_as_client == TRUE)
        {
        if(init_taskmaster_client(taskmaster) != EXIT_SUCCESS)
            {
            free_program_list(&(taskmaster->programs));
            free_command_line(&(taskmaster->command_line));
            return (EXIT_FAILURE);
            }
        }

    taskmaster->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }

void    stop_and_wait_all_the_program(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return;
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return;
    if(taskmaster->programs.global_status.global_status_struct_init == FALSE)
        return;

    struct program_specification *actual_program;
    uint32_t                      number_of_seconds;
    uint32_t                      tmp_number_of_program;
    uint8_t                       buffer[OUTPUT_BUFFER_SIZE];

    actual_program        = NULL;
    buffer[0]             = NIL;
    number_of_seconds     = 0;
    tmp_number_of_program = 0;

    actual_program = taskmaster->programs.program_linked_list;
    while(actual_program != NULL)
        {
        if(actual_program->global_status.global_status_struct_init == FALSE)
            {
            actual_program = NULL;
            break;
            }

        actual_program->program_state.need_to_be_removed = TRUE;

        actual_program = actual_program->next;
        }

    //TODO wait for all the program to stop
    tmp_number_of_program = taskmaster->programs.number_of_program;
    number_of_seconds     = 0;
    while(taskmaster->programs.number_of_program != 0 && number_of_seconds < NUMBER_OF_SECONDS_TO_WAIT_FOR_EXIT)
        {
        sleep(1);
        if(taskmaster->programs.number_of_program > 0)
            {
            //TODO during wait display status of all the program or number of program still up?
            snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Number of program still running (%u/%u)\n", taskmaster->programs.number_of_program, tmp_number_of_program);
            print_command_output(taskmaster, buffer);
            }
        number_of_seconds++;
        }
    }

void free_taskmaster(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return;
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return;

    //TODO set all the program to remove
    //TODO wait for all the program to stop
    stop_and_wait_all_the_program(taskmaster);
    free_program_list(&(taskmaster->programs));

    free_command_line(&(taskmaster->command_line));

    if(taskmaster->socket != -1)
        close(taskmaster->socket);
    if(taskmaster->client_socket != -1)
        close(taskmaster->client_socket);

    taskmaster->programs.global_status.global_status_struct_init = FALSE;
    }
