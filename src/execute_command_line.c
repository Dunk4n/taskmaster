#include "taskmaster.h"
#include "minishell.h"

/**
* This array correspond to all the possible shell command name
*/
const uint8_t *g_shell_command_name[NUMBER_OF_SHELL_COMMAND] =
    {
    (uint8_t *) "status",
    (uint8_t *) "start",
    (uint8_t *) "stop",
    (uint8_t *) "restart",
    (uint8_t *) "reload_conf",
    (uint8_t *) "exit",
    };

/**
* This array correspond to all the possible shell command name function
*/
uint8_t (*const g_shell_command_function[NUMBER_OF_SHELL_COMMAND])(struct taskmaster *taskmaster, uint8_t **arguments) =
    {
    shell_command_status_function,
    shell_command_start_function,
    shell_command_stop_function,
    shell_command_restart_function,
    shell_command_reload_conf_function,
    shell_command_exit_function,
    };

uint8_t *get_next_argument(char *line, int32_t *id)
    {
    if(line == NULL)
        return (NULL);
    if(id == NULL)
        return (NULL);

    uint32_t  cnt;
    uint32_t  length;
    uint8_t  *argument;
    uint8_t   not_empty;

    cnt         = 0;
    argument = NULL;
    length      = 0;
    not_empty   = FALSE;

    cnt = 0;
    while(line[cnt] == ' ' || line[cnt] == '\t')
        cnt++;
    *id += cnt;
    line += cnt;

    length = 0;
    cnt = 0;
    while(line[cnt] != NIL && line[cnt] != ' ' && line[cnt] != '\t')
        {
        if(line[cnt] == '\'')
            {
            not_empty = TRUE;
            cnt++;
            while(line[cnt] != '\'')
                {
                length++;
                cnt++;
                }
            cnt++;
            }
        else if(line[cnt] == '"')
            {
            not_empty = TRUE;
            cnt++;
            while(line[cnt] != '"')
                {
                length++;
                cnt++;
                }
            cnt++;
            }
        else if(line[cnt] == '\\')
            {
            cnt++;
            if(line[cnt] != NIL)
                {
                cnt++;
                length++;
                }
            }
        else
            {
            length++;
            cnt++;
            }
        }

    if(not_empty == FALSE && length == 0)
        return (NULL);
    if(length == UINT32_MAX)
        return (NULL);

    argument = malloc(sizeof(uint8_t) * (length + 1));
    if(argument == NULL)
        return (NULL);
    argument[length] = NIL;

    length = 0;
    cnt = 0;
    while(line[cnt] != NIL && line[cnt] != ' ' && line[cnt] != '\t')
        {
        if(line[cnt] == '\'')
            {
            cnt++;
            while(line[cnt] != '\'')
                {
                argument[length] = line[cnt];
                length++;
                cnt++;
                }
            cnt++;
            }
        else if(line[cnt] == '"')
            {
            cnt++;
            while(line[cnt] != '"')
                {
                argument[length] = line[cnt];
                length++;
                cnt++;
                }
            cnt++;
            }
        else if(line[cnt] == '\\')
            {
            cnt++;
            if(line[cnt] != NIL)
                {
                argument[length] = line[cnt];
                cnt++;
                length++;
                }
            }
        else
            {
            argument[length] = line[cnt];
            length++;
            cnt++;
            }
        }
    argument[length] = NIL;

    *id += cnt;
    line += cnt;

    cnt = 0;
    while(line[cnt] == ' ' || line[cnt] == '\t')
        cnt++;
    *id += cnt;

    return (argument);
    }

uint8_t separate_line_in_argument(char *line, uint8_t **arguments)
    {
    if(line == NULL)
        return (EXIT_FAILURE);
    if(arguments == NULL)
        return (EXIT_FAILURE);
    if(SHELL_MAX_ARGUMENT == 0 || SHELL_MAX_ARGUMENT >= UINT8_MAX)
        return (EXIT_FAILURE);

    int8_t  cnt;
    int32_t id;

    id = 0;

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT)
        {
        arguments[cnt] = get_next_argument(line + id, &id);
        if(arguments[cnt] == NULL)
            return (EXIT_SUCCESS);
        cnt++;
        }

    return (EXIT_SUCCESS);
    }

uint8_t execute_command_line(struct taskmaster *taskmaster, char *line)
    {
    if(taskmaster == NULL)
        return (EXIT_FAILURE);
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return (EXIT_FAILURE);
    if(line == NULL)
        return (EXIT_FAILURE);
    if(SHELL_MAX_ARGUMENT == 0 || SHELL_MAX_ARGUMENT >= UINT8_MAX)
        return (EXIT_FAILURE);

    uint8_t  buffer[OUTPUT_BUFFER_SIZE];
    uint8_t  cnt;
    uint8_t *arguments[SHELL_MAX_ARGUMENT + 1];

    buffer[0] = NIL;

    cnt = 0;
    while(cnt <= SHELL_MAX_ARGUMENT)
        {
        arguments[cnt] = NULL;
        cnt++;
        }

    if(separate_line_in_argument(line, arguments) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(arguments[0] == NULL)
        {
        cnt = 0;
        while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
            {
            free(arguments[cnt]);
            arguments[cnt] = NULL;
            cnt++;
            }
        return (EXIT_SUCCESS);
        }

    cnt = 0;
    while(cnt < NUMBER_OF_SHELL_COMMAND && cnt < UINT8_MAX)
        {
        if(strcmp((char *) arguments[0], (char *) g_shell_command_name[cnt]) == 0)
            {
            if(g_shell_command_function[cnt](taskmaster, arguments) != EXIT_SUCCESS)
                {
                //TODO echo command failed
                //TODO display help?
                snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Command failed!\n");
                print_command_output(taskmaster, buffer);
                }

            break;
            }

        cnt++;
        }

    if(cnt >= NUMBER_OF_SHELL_COMMAND)
        {
        snprintf((char *) buffer, OUTPUT_BUFFER_SIZE, "Command not found\n");
        print_command_output(taskmaster, buffer);
        //TODO echo command not found
        //TODO display commands help?
        }

    cnt = 0;
    while(cnt < SHELL_MAX_ARGUMENT && arguments[cnt] != NULL)
        {
        free(arguments[cnt]);
        arguments[cnt] = NULL;
        cnt++;
        }

    return (EXIT_SUCCESS);
    }
