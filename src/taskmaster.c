#include "taskmaster.h"
#include "minishell.h"

uint8_t send_reload_config_file_to_daemon(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->command_line.global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(taskmaster->config_file_path == NULL)
        return (EXIT_FAILURE);

    uint32_t  cnt;
    uint32_t  length;
    uint8_t  *str_reload_config_command;

    cnt = 0;
    length = 0;
    str_reload_config_command = NULL;

    cnt = 0;
    while(taskmaster->config_file_path[cnt] != NIL)
        cnt++;
    length = cnt;

    cnt = 0;
    while(g_shell_command_name[SHELL_COMMAND_RELOAD_CONF][cnt] != NIL)
        cnt++;
    length += cnt;
    length++;

    str_reload_config_command = malloc(sizeof(uint8_t) * (length + 1));
    if(str_reload_config_command == NULL)
        return (EXIT_FAILURE);

    length = 0;
    cnt = 0;
    while(g_shell_command_name[SHELL_COMMAND_RELOAD_CONF][cnt] != NIL)
        {
        str_reload_config_command[length] = g_shell_command_name[SHELL_COMMAND_RELOAD_CONF][cnt];

        cnt++;
        length++;
        }

    str_reload_config_command[length] = ' ';
    length++;

    while(taskmaster->config_file_path[cnt] != NIL)
        {
        str_reload_config_command[length] = taskmaster->config_file_path[cnt];

        cnt++;
        length++;
        }
    str_reload_config_command[length] = NIL;

    if(send_command_line_to_daemon(taskmaster, (char *) str_reload_config_command) != EXIT_SUCCESS)
        {
        free(str_reload_config_command);
        str_reload_config_command = NULL;
        return (EXIT_FAILURE);
        }

    free(str_reload_config_command);
    str_reload_config_command = NULL;

    return (EXIT_SUCCESS);
    }

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

        if(g_config_must_reload == TRUE)
            {
            if(taskmaster->global_status.global_status_start_as_client == TRUE)
                {
                if(send_reload_config_file_to_daemon(taskmaster) != EXIT_SUCCESS)
                    return (EXIT_FAILURE);
                }
            else
                {
                if(reload_config_file(taskmaster->config_file_path, &(taskmaster->programs)) != EXIT_SUCCESS)
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
                }

            g_config_must_reload = FALSE;
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

    taskmaster->config_file_path = NULL;

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

    if(pthread_mutex_lock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return;

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

    if(pthread_mutex_unlock(&(taskmaster->programs.mutex_program_linked_list)) != 0)
        return;

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

    free(taskmaster->config_file_path);

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
