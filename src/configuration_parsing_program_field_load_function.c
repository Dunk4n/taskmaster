#include "taskmaster.h"

/**
* This function convert a string to an unsigned int
*/
unsigned int atoui(const char *str)
{
    unsigned int i;
    long         nb;

    if(str == NULL)
        return (0);

    i = 0;
    nb = 0;
    while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
        i++;
    while (str[i] >= '0' && str[i] <= '9')
        nb = (unsigned int)(nb * 10 + str[i++] - '0');
    return (nb);
}

/**
* This function parse the attribute cmd in an object program in the config file during the parsing
*/
uint8_t program_field_cmd_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t *new_cmd;
    uint32_t cnt;

    new_cmd = NULL;
    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    new_cmd = reallocarray(program->str_start_command, sizeof(uint8_t), event->data.scalar.length + 1);
    if(new_cmd == NULL)
        return (RETURN_FAILURE);

    program->str_start_command = new_cmd;

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        program->str_start_command[cnt] = event->data.scalar.value[cnt];
        cnt++;
        }
    program->str_start_command[cnt] = NIL;

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute numprocs in an object program in the config file during the parsing
*/
uint8_t program_field_numprocs_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t cnt;

    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
            return (RETURN_FAILURE);

        cnt++;
        }

    program->number_of_process = atoui((char *) event->data.scalar.value);
    if(program->number_of_process == 0)
        return (RETURN_FAILURE);
    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute numprocs in an object program in the config file during the parsing
*/
uint8_t program_field_autostart_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    if(strcmp((char *) event->data.scalar.value, "false") == 0)
        program->auto_start = FALSE;
    else if(strcmp((char *) event->data.scalar.value, "true") == 0)
        program->auto_start = TRUE;
    else
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute autorestart in an object program in the config file during the parsing
*/
uint8_t program_field_autorestart_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    if(strcmp((char *) event->data.scalar.value, "false") == 0)
        program->e_auto_restart = PROGRAM_AUTO_RESTART_FALSE;
    else if(strcmp((char *) event->data.scalar.value, "true") == 0)
        program->e_auto_restart = PROGRAM_AUTO_RESTART_TRUE;
    else if(strcmp((char *) event->data.scalar.value, "unexpected") == 0)
        program->e_auto_restart = PROGRAM_AUTO_RESTART_UNEXPECTED;
    else
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute exitcodes in an object program in the config file during the parsing
*/
uint8_t program_field_exitcodes_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t cnt;
    uint32_t pos;
    uint32_t exit_code;
    uint8_t *tmp_exit_codes;

    cnt            = 0;
    pos            = 0;
    exit_code      = 0;
    tmp_exit_codes = NULL;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type == YAML_SCALAR_EVENT)
        {
        if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
            return (RETURN_FAILURE);

        exit_code = 0;

        cnt = 0;
        while(cnt < event->data.scalar.length)
            {
            if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
                return (RETURN_FAILURE);

            cnt++;
            }

        exit_code = atoui((char *) event->data.scalar.value);
        if(exit_code > UINT8_MAX)
            return (RETURN_FAILURE);

        tmp_exit_codes = realloc(program->exit_codes, 1 * sizeof(uint8_t));
        if(tmp_exit_codes == NULL)
            return (RETURN_FAILURE);

        program->exit_codes = tmp_exit_codes;

        program->exit_codes_number = 1;

        program->exit_codes[0] = exit_code;
        }
    else if(event->type == YAML_SEQUENCE_START_EVENT)
        {
        pos = 0;
        while(TRUE)
            {
            yaml_event_delete(event);
            if(yaml_parser_parse(parser, event) != 1)
                {
                #ifdef DEVELOPEMENT
                fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
                #endif

                #ifdef DEMO
                fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
                #endif

                #ifdef PRODUCTION
                fprintf(stderr, "\033[1;31mERROR\033[0m\n");
                #endif 
                return (RETURN_FAILURE);
                }

            if(event->type == YAML_SCALAR_EVENT)
                {
                if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
                    return (RETURN_FAILURE);

                exit_code = 0;

                cnt = 0;
                while(cnt < event->data.scalar.length)
                    {
                    if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
                        return (RETURN_FAILURE);

                    cnt++;
                    }

                exit_code = atoui((char *) event->data.scalar.value);
                if(exit_code > UINT8_MAX)
                    return (RETURN_FAILURE);

                cnt = 0;
                while(cnt < program->exit_codes_number)
                    {
                    if(exit_code == program->exit_codes[cnt])
                        break;
                    cnt++;
                    }

                if(cnt == program->exit_codes_number)
                    {
                    tmp_exit_codes = reallocarray(program->exit_codes, sizeof(uint8_t), pos + 1);
                    if(tmp_exit_codes == NULL)
                        return (RETURN_FAILURE);

                    program->exit_codes = tmp_exit_codes;

                    program->exit_codes_number = pos + 1;

                    program->exit_codes[pos] = exit_code;

                    pos++;
                    }
                }
            else if(event->type == YAML_SEQUENCE_END_EVENT)
                break;
            else
                return (RETURN_FAILURE);
            }
        }
    else
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute starttime in an object program in the config file during the parsing
*/
uint8_t program_field_starttime_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t cnt;

    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
            return (RETURN_FAILURE);

        cnt++;
        }

    program->start_time = atoui((char *) event->data.scalar.value);
    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute startretries in an object program in the config file during the parsing
*/
uint8_t program_field_startretries_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t cnt;

    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
            return (RETURN_FAILURE);

        cnt++;
        }

    program->start_retries = atoui((char *) event->data.scalar.value);
    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute stopsignal in an object program in the config file during the parsing
*/
uint8_t program_field_stopsignal_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t *signal_str_name[] =
        {
        (uint8_t *) "HUP",
        (uint8_t *) "INT",
        (uint8_t *) "QUIT",
        (uint8_t *) "KILL",
        (uint8_t *) "USR1",
        (uint8_t *) "USR2",
        (uint8_t *) "TERM"
        };

    int32_t signal_value[] =
        {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGKILL,
        SIGUSR1,
        SIGUSR2,
        SIGTERM
        };

    int32_t value;
    uint8_t cnt;

    cnt = 0;
    value = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    value = 0;
    value = (int32_t) atoui((char *) event->data.scalar.value);

    cnt = 0;
    while(cnt < sizeof(signal_value) / sizeof(int32_t))
        {
        if(value == signal_value[cnt])
            {
            program->stop_signal = signal_value[cnt];
            return (RETURN_SUCCESS);
            }

        cnt++;
        }

    cnt = 0;
    while(cnt < sizeof(signal_str_name) / sizeof(uint8_t *))
        {
        if(strcmp((char *) event->data.scalar.value, (char *) signal_str_name[cnt]) == 0)
            {
            program->stop_signal = signal_value[cnt];
            return (RETURN_SUCCESS);
            }

        cnt++;
        }

    return (RETURN_FAILURE);
}

/**
* This function parse the attribute stoptime in an object program in the config file during the parsing
*/
uint8_t program_field_stoptime_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t cnt;

    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '9')
            return (RETURN_FAILURE);

        cnt++;
        }

    program->stop_time = atoui((char *) event->data.scalar.value);
    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute stdout in an object program in the config file during the parsing
*/
uint8_t program_field_stdout_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t *new_cmd;
    uint32_t cnt;

    new_cmd = NULL;
    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    new_cmd = reallocarray(program->str_stdout, sizeof(uint8_t), event->data.scalar.length + 1);
    if(new_cmd == NULL)
        return (RETURN_FAILURE);

    program->str_stdout = new_cmd;

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        program->str_stdout[cnt] = event->data.scalar.value[cnt];
        cnt++;
        }
    program->str_stdout[cnt] = NIL;

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute stderr in an object program in the config file during the parsing
*/
uint8_t program_field_stderr_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t *new_cmd;
    uint32_t cnt;

    new_cmd = NULL;
    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    new_cmd = reallocarray(program->str_stderr, sizeof(uint8_t), event->data.scalar.length + 1);
    if(new_cmd == NULL)
        return (RETURN_FAILURE);

    program->str_stderr = new_cmd;

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        program->str_stderr[cnt] = event->data.scalar.value[cnt];
        cnt++;
        }
    program->str_stderr[cnt] = NIL;
    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute env in an object program in the config file during the parsing
*/
uint8_t program_field_env_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint32_t  cnt;
    uint32_t  pos;
    uint32_t  size;
    uint8_t  *tmp_str;
    uint8_t **tmp_env;

    cnt     = 0;
    pos     = 0;
    size    = 0;
    tmp_env = NULL;
    tmp_str = NULL;

    if(program->env != NULL)
        {
        cnt = 0;
        while(cnt < program->env_length)
            {
            free(program->env[cnt]);
            program->env[cnt] = NULL;
            cnt++;
            }

        free(program->env);
        }
    program->env = NULL;
    program->env_length = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
        return (RETURN_FAILURE);

    pos = 0;
    while(TRUE)
        {
        yaml_event_delete(event);
        if(yaml_parser_parse(parser, event) != 1)
            {
            #ifdef DEVELOPEMENT
            fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
            #endif

            #ifdef DEMO
            fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
            #endif

            #ifdef PRODUCTION
            fprintf(stderr, "\033[1;31mERROR\033[0m\n");
            #endif 
            return (RETURN_FAILURE);
            }

        if(event->type == YAML_SCALAR_EVENT)
            {
            if(event->data.scalar.length == 0 || event->data.scalar.value == NULL)
                return (RETURN_FAILURE);
            if(event->data.scalar.length >= SIZE_MAX - 1)
                return (RETURN_FAILURE);

            if(pos == UINT32_MAX)
                return (RETURN_FAILURE);

            tmp_env = NULL;
            tmp_env = reallocarray(program->env, sizeof(uint8_t *), pos + 1);
            if(tmp_env == NULL)
                return (RETURN_FAILURE);
            program->env = tmp_env;
            program->env[pos] = NULL;

            program->env_length = pos + 1;

            program->env[pos] = malloc((event->data.scalar.length + 2) * sizeof(uint8_t));
            if(program->env[pos] == NULL)
                return (RETURN_FAILURE);

            cnt = 0;
            while(cnt < event->data.scalar.length)
                {
                program->env[pos][cnt] = event->data.scalar.value[cnt];
                cnt++;
                }
            program->env[pos][cnt] = '=';
            cnt++;
            program->env[pos][cnt] = NIL;

            size = cnt;

            yaml_event_delete(event);
            if(yaml_parser_parse(parser, event) != 1)
                {
                #ifdef DEVELOPEMENT
                fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
                #endif

                #ifdef DEMO
                fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
                #endif

                #ifdef PRODUCTION
                fprintf(stderr, "\033[1;31mERROR\033[0m\n");
                #endif 
                return (RETURN_FAILURE);
                }

            if(event->type != YAML_SCALAR_EVENT || event->data.scalar.length == 0 || event->data.scalar.value == NULL)
                return (RETURN_FAILURE);

            if(event->data.scalar.length == SIZE_MAX)
                return (RETURN_FAILURE);
            if(cnt >= SIZE_MAX - 1 - event->data.scalar.length)
                return (RETURN_FAILURE);

            tmp_str = NULL;
            tmp_str = realloc(program->env[pos], (cnt + 1 + event->data.scalar.length) * sizeof(uint8_t));
            if(tmp_str == NULL)
                return (RETURN_FAILURE);
            program->env[pos] = tmp_str;

            cnt = 0;
            while(cnt < event->data.scalar.length)
                {
                program->env[pos][size + cnt] = event->data.scalar.value[cnt];
                cnt++;
                }
            program->env[pos][size + cnt] = NIL;

            pos++;
            }
        else if(event->type == YAML_MAPPING_END_EVENT)
            break;
        else
            return (RETURN_FAILURE);
        }

    if(event->type != YAML_MAPPING_END_EVENT)
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute workingdir in an object program in the config file during the parsing
*/
uint8_t program_field_workingdir_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t *new_cmd;
    uint32_t cnt;

    new_cmd = NULL;
    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    new_cmd = reallocarray(program->str_working_directory, sizeof(uint8_t), event->data.scalar.length + 1);
    if(new_cmd == NULL)
        return (RETURN_FAILURE);

    program->str_working_directory = new_cmd;

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        program->str_working_directory[cnt] = event->data.scalar.value[cnt];
        cnt++;
        }
    program->str_working_directory[cnt] = NIL;

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute umask in an object program in the config file during the parsing
*/
uint8_t program_field_umask_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    uint8_t cnt;

    cnt = 0;

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    if(event->data.scalar.length != 3)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < event->data.scalar.length)
        {
        if(event->data.scalar.value[cnt] < '0' || event->data.scalar.value[cnt] > '7')
            return (RETURN_FAILURE);

        cnt++;
        }

    program->umask = 0;
    program->umask = (event->data.scalar.value[0] - '0') << 6 | (event->data.scalar.value[1] - '0') << 3 | (event->data.scalar.value[2] - '0');

    return (RETURN_SUCCESS);
}

/**
* This function parse the attribute log in an object program in the config file during the parsing
*/
uint8_t program_field_log_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
{
    if(parser == NULL)
        return (RETURN_FAILURE);

    if(program == NULL)
        return (RETURN_FAILURE);

    if(event == NULL)
        return (RETURN_FAILURE);

    yaml_event_delete(event);
    if(yaml_parser_parse(parser, event) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the YAML config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif 
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_SCALAR_EVENT || event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    if(strcmp((char *) event->data.scalar.value, "false") == 0)
        program->e_log = PROGRAM_LOG_FALSE;
    else if(strcmp((char *) event->data.scalar.value, "true") == 0)
        program->e_log = PROGRAM_LOG_TRUE;
    else if(strcmp((char *) event->data.scalar.value, "mail") == 0)
        program->e_log = PROGRAM_LOG_MAIL;
    else
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
}
