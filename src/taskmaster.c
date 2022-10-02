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

        if(execute_command_line(taskmaster, line) != EXIT_SUCCESS)
            {
            free(line);
            line = NULL;
            return (EXIT_FAILURE);
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

    char *str_term_value;

    str_term_value = NULL;

    taskmaster->programs.global_status.global_status_struct_init = FALSE;

    memset(&taskmaster->global_status, 0, sizeof(taskmaster->global_status));
    taskmaster->global_status.global_status_exit = FALSE;

    memset(taskmaster->str_line, 0, LINE_SIZE);
    taskmaster->line_size = 0;
    taskmaster->pos_in_line = 0;
    taskmaster->cursor_pos_x = 0;
    taskmaster->cursor_pos_y = 0;

    taskmaster->command_line.global_status.global_status_struct_init = FALSE;
    if(init_command_line(&(taskmaster->command_line)) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(init_program_list(&(taskmaster->programs)) != EXIT_SUCCESS)
        {
        free_command_line(&(taskmaster->command_line));
        return (EXIT_FAILURE);
        }

    taskmaster->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }

void free_taskmaster(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return;
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return;

    memset(taskmaster->str_line, 0, LINE_SIZE);
    taskmaster->line_size = 0;
    taskmaster->pos_in_line = 0;
    taskmaster->cursor_pos_x = 0;
    taskmaster->cursor_pos_y = 0;

    free_program_list(&(taskmaster->programs));

    free_command_line(&(taskmaster->command_line));

    taskmaster->programs.global_status.global_status_struct_init = FALSE;
    }
