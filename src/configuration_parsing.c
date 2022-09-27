#include "taskmaster.h"

/**
* This array correspond to all the possible attribut name for a program in the YAML config file
*/
const uint8_t *g_program_specification_field_name[NUMBER_OF_PROGRAM_SPECIFICATION_FIELD] =
    {
    (uint8_t *) "cmd",
    (uint8_t *) "numprocs",
    (uint8_t *) "autostart",
    (uint8_t *) "autorestart",
    (uint8_t *) "exitcodes",
    (uint8_t *) "starttime",
    (uint8_t *) "startretries",
    (uint8_t *) "stopsignal",
    (uint8_t *) "stoptime",
    (uint8_t *) "stdout",
    (uint8_t *) "stderr",
    (uint8_t *) "env",
    (uint8_t *) "workingdir",
    (uint8_t *) "umask",
    (uint8_t *) "log",
    };

/**
* This array correspond to all the possible attribut parsing function for a program in the YAML config file
*/
uint8_t (*const g_program_specification_field_load_function[NUMBER_OF_PROGRAM_SPECIFICATION_FIELD])(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event) =
    {
    program_field_cmd_load_function,
    program_field_numprocs_load_function,
    program_field_autostart_load_function,
    program_field_autorestart_load_function,
    program_field_exitcodes_load_function,
    program_field_starttime_load_function,
    program_field_startretries_load_function,
    program_field_stopsignal_load_function,
    program_field_stoptime_load_function,
    program_field_stdout_load_function,
    program_field_stderr_load_function,
    program_field_env_load_function,
    program_field_workingdir_load_function,
    program_field_umask_load_function,
    program_field_log_load_function,
    };

/**
* This function set to the default value the structure program_specification
*/
uint8_t set_program_default_value(struct program_specification *program)
    {
    if(program == NULL)
        {
        return (RETURN_FAILURE);
        }

    memset(&program->global_status, 0, sizeof(program->global_status));
    program->global_status.global_status_struct_init             = FALSE;
    program->global_status.global_status_conf_loaded             = FALSE;
    program->global_status.global_status_configuration_reloading = FALSE;
    program->global_status.global_status_need_to_restart         = FALSE;

    program->str_name = NULL;
    program->name_length = 0;
    program->str_start_command = NULL;
    program->number_of_process = PROGRAM_DEFAULT_NUMBER_OF_PROCESS;
    program->auto_start = PROGRAM_DEFAULT_AUTO_START;
    program->e_auto_restart = PROGRAM_DEFAULT_AUTO_RESTART;
    program->exit_codes = NULL;
    program->start_time = PROGRAM_DEFAULT_START_TIME;
    program->start_retries = PROGRAM_DEFAULT_START_RETRIES;
    program->stop_signal = PROGRAM_DEFAULT_STOP_SIGNAL;
    program->stop_time = PROGRAM_DEFAULT_STOP_TIME;
    program->str_stdout = NULL;
    program->str_stderr = NULL;
    program->env = NULL;
    program->env_length = 0;
    program->str_working_directory = NULL;
    program->umask = PROGRAM_DEFAULT_UMASk;
    program->e_log = PROGRAM_DEFAULT_LOG_STATUS;

    program->exit_codes = malloc(1 * sizeof(uint8_t));
    if(program->exit_codes == NULL)
        return (RETURN_FAILURE);
    program->exit_codes[0] = PROGRAM_DEFAULT_EXIT_CODE;
    program->exit_codes_number = 1;

    program->global_status.global_status_struct_init = TRUE;

    return (RETURN_SUCCESS);
    }

/**
* This function reset to the default value the structure program_specification without the name
*/
uint8_t reset_program_default_value_without_name(struct program_specification *program)
    {
    if(program == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program->global_status.global_status_struct_init == FALSE)
        {
        return (RETURN_FAILURE);
        }

    uint32_t cnt;

    cnt = 0;

    memset(&program->global_status, 0, sizeof(program->global_status));
    program->global_status.global_status_struct_init             = FALSE;
    program->global_status.global_status_conf_loaded             = FALSE;
    program->global_status.global_status_configuration_reloading = FALSE;
    program->global_status.global_status_need_to_restart         = FALSE;

    free(program->str_start_command);
    program->str_start_command = NULL;

    program->number_of_process = PROGRAM_DEFAULT_NUMBER_OF_PROCESS;

    program->auto_start = PROGRAM_DEFAULT_AUTO_START;
    program->e_auto_restart = PROGRAM_DEFAULT_AUTO_RESTART;

    free(program->exit_codes);
    program->exit_codes = NULL;

    program->start_time = PROGRAM_DEFAULT_START_TIME;
    program->start_retries = PROGRAM_DEFAULT_START_RETRIES;
    program->stop_signal = PROGRAM_DEFAULT_STOP_SIGNAL;
    program->stop_time = PROGRAM_DEFAULT_STOP_TIME;
    free(program->str_stdout);
    program->str_stdout = NULL;
    free(program->str_stderr);
    program->str_stderr = NULL;

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

    free(program->str_working_directory);
    program->str_working_directory = NULL;
    program->umask = PROGRAM_DEFAULT_UMASk;
    program->e_log = PROGRAM_DEFAULT_LOG_STATUS;

    program->exit_codes = malloc(1 * sizeof(uint8_t));
    if(program->exit_codes == NULL)
        return (RETURN_FAILURE);
    program->exit_codes[0] = PROGRAM_DEFAULT_EXIT_CODE;
    program->exit_codes_number = 1;

    program->global_status.global_status_struct_init = TRUE;

    return (RETURN_SUCCESS);
    }

/**
* This function add a new structure program_specification in the dynamic array of structure program_specification in the structure program_list
*/
uint8_t add_new_program(struct program_list *program_list, uint8_t *program_name, size_t program_name_length)
    {
    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_name == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_name_length == 0 || program_name_length == SIZE_MAX)
        {
        return (RETURN_FAILURE);
        }

    if(program_list->number_of_program == UINT32_MAX)
        {
        return (RETURN_FAILURE);
        }

    struct program_specification *programs_tmp;
    size_t                        cnt;

    programs_tmp = NULL;
    cnt          = 0;

    programs_tmp = reallocarray(program_list->programs, sizeof(struct program_specification), program_list->number_of_program + 1);

    if(programs_tmp == NULL)
        return (RETURN_FAILURE);

    program_list->programs = programs_tmp;

    program_list->number_of_program++;

    if(set_program_default_value(&(program_list->programs[program_list->number_of_program - 1])) != RETURN_SUCCESS)
        return (RETURN_FAILURE);

    program_list->programs[program_list->number_of_program - 1].str_name = malloc((program_name_length + 1) * sizeof(uint8_t));
    if(program_list->programs[program_list->number_of_program - 1].str_name == NULL)
        return (RETURN_FAILURE);

    cnt = 0;
    while(cnt < program_name_length)
        {
        program_list->programs[program_list->number_of_program - 1].str_name[cnt] = program_name[cnt];

        cnt++;
        }

    program_list->programs[program_list->number_of_program - 1].str_name[cnt] = NIL;
    program_list->programs[program_list->number_of_program - 1].name_length = cnt;

    return (RETURN_SUCCESS);
    }

/**
* This function check if all the required attribut of the structure program_specification has been set
*/
uint8_t check_required_attribute_in_program(struct program_specification *program)
    {
    if(program == NULL)
        return (RETURN_FAILURE);

    if(program->str_name == NULL || program->name_length == 0)
        return (RETURN_FAILURE);
    if(program->str_start_command == NULL)
        return (RETURN_FAILURE);
    if(program->exit_codes == NULL || program->exit_codes_number == 0)
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
    }

/**
* This function parse all the attribute of one program in the YAML config file
*/
uint8_t parse_config_program_attribute(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    uint8_t cnt;

    cnt = 0;

    if(event->type != YAML_MAPPING_START_EVENT)
        return (RETURN_FAILURE);

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

        if(event->type == YAML_MAPPING_END_EVENT)
            break;

        if(event->type == YAML_SCALAR_EVENT && event->data.scalar.value != NULL && event->data.scalar.length != 0)
            {
            cnt = 0;
            while(cnt < NUMBER_OF_PROGRAM_SPECIFICATION_FIELD)
                {
                if(strcmp((char *) event->data.scalar.value, (char *) g_program_specification_field_name[cnt]) == 0)
                    {
                    if(g_program_specification_field_load_function[cnt](parser, program, event) != RETURN_SUCCESS)
                        return (RETURN_FAILURE);

                    break;
                    }

                cnt++;
                }
            }
        else
            return (RETURN_FAILURE);
        }

    if(check_required_attribute_in_program(program) != RETURN_SUCCESS)
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
    }

/**
* This function parse one program in the YAML config file
*/
uint8_t parse_config_one_program(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    struct program_specification *actual_program_specification;
    uint32_t                      cnt;

    actual_program_specification = NULL;
    cnt                          = 0;

    if(event->type != YAML_SCALAR_EVENT)
        return (RETURN_FAILURE);

    if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (RETURN_FAILURE);

    if(program_list->programs != NULL && program_list->number_of_program > 0)
        {
        actual_program_specification = NULL;
        cnt = 0;
        while(cnt < program_list->number_of_program)
            {
            if(program_list->programs[cnt].global_status.global_status_struct_init == TRUE && program_list->programs[cnt].name_length != 0 && program_list->programs[cnt].str_name != NULL && strcmp((char *) event->data.scalar.value, (char *) program_list->programs[cnt].str_name) == 0)
                {
                actual_program_specification = &(program_list->programs[cnt]);
                if(reset_program_default_value_without_name(actual_program_specification) != RETURN_SUCCESS)
                    return (RETURN_FAILURE);
                actual_program_specification->global_status.global_status_configuration_reloading = TRUE;
                break;
                }

            cnt++;
            }
        }

    if(actual_program_specification == NULL)
        {
        if(add_new_program(program_list, event->data.scalar.value, event->data.scalar.length) != RETURN_SUCCESS)
            return (RETURN_FAILURE);

        actual_program_specification = &(program_list->programs[program_list->number_of_program - 1]);
        }

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

    if(parse_config_program_attribute(parser, actual_program_specification, event) != RETURN_SUCCESS)
        return (RETURN_FAILURE);

    if(event->type != YAML_MAPPING_END_EVENT)
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
    }

/**
* This function parse all the programs in the YAML config file
*/
uint8_t parse_config_all_programs(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
        return (RETURN_FAILURE);

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

        switch(event->type)
            {
            case(YAML_MAPPING_END_EVENT):
                return (RETURN_SUCCESS);
            case(YAML_SCALAR_EVENT):
                if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
                    return (RETURN_FAILURE);
                if(parse_config_one_program(parser, program_list, event) != RETURN_SUCCESS)
                    return (RETURN_FAILURE);
                break;
            default:
                return (RETURN_FAILURE);
            }
        }

    return (RETURN_SUCCESS);
    }

/**
* This function parse the mapping event of the name programs in the YAML config file
*/
uint8_t parse_config_mapping_event_programs(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
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

    switch(event->type)
        {
        case(YAML_MAPPING_END_EVENT):
            return (RETURN_SUCCESS);
        case(YAML_SCALAR_EVENT):
            if(event->data.scalar.value == NULL || event->data.scalar.length == 0 || strcmp((char *) event->data.scalar.value, PROGRAM_CATEGORY_STR) != 0)
                return (RETURN_FAILURE);

            program_list->programs_loaded = TRUE;

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

            switch(event->type)
                {
                case(YAML_MAPPING_END_EVENT):
                    return (RETURN_SUCCESS);
                case(YAML_MAPPING_START_EVENT):
                    if(parse_config_all_programs(parser, program_list, event) != RETURN_SUCCESS)
                        return (RETURN_FAILURE);
                    break;
                default:
                    return (RETURN_FAILURE);
                }
            break;
        default:
            return (RETURN_FAILURE);
        }

    return (RETURN_SUCCESS);
    }

/**
* This function parse the document event in the YAML config file
*/
uint8_t parse_config_document_event(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_DOCUMENT_START_EVENT)
        return (RETURN_FAILURE);

    while(program_list->programs_loaded == FALSE)
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

        switch(event->type)
            {
            case(YAML_DOCUMENT_END_EVENT):
                return (RETURN_SUCCESS);
            case(YAML_MAPPING_START_EVENT):
                if(parse_config_mapping_event_programs(parser, program_list, event) != RETURN_SUCCESS)
                    return (RETURN_FAILURE);
                break;
            default:
                return (RETURN_FAILURE);
            }
        }

    return (RETURN_SUCCESS);
    }

/**
* This function parse the stream event in the YAML config file
*/
uint8_t parse_config_stream_event(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(event->type != YAML_STREAM_START_EVENT)
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

    if(event->type == YAML_STREAM_END_EVENT)
        return (RETURN_SUCCESS);

    if(event->type == YAML_DOCUMENT_START_EVENT)
        {
        if(parse_config_document_event(parser, program_list, event) != RETURN_SUCCESS)
            return (RETURN_FAILURE);
        }
    else
        return (RETURN_FAILURE);

    return (RETURN_SUCCESS);
    }

/**
* This function parse the YAML config file and store the content in the structure program_list
*/
uint8_t parse_config_file(uint8_t *file_name, struct program_list *program_list)
    {
    if(file_name == NULL)
        {
        return (RETURN_FAILURE);
        }

    if(program_list == NULL)
        {
        return (RETURN_FAILURE);
        }

    FILE          *file;
    yaml_parser_t  parser;
    yaml_event_t   event;

    file = NULL;
    file = fopen((char *) file_name, "r");
    if(file == NULL)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The config file can not be opened\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif
        return (RETURN_FAILURE);
        }

    if(yaml_parser_initialize(&parser) != 1)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to initialize the YAML parser failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif

        fclose(file);
        return (RETURN_FAILURE);
        }

    yaml_parser_set_input_file(&parser, file);

    if(yaml_parser_parse(&parser, &event) != 1)
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

        fclose(file);
        yaml_parser_delete(&parser);
        return (RETURN_FAILURE);
        }

    if(event.type != YAML_STREAM_START_EVENT)
        {
        yaml_event_delete(&event);
        fclose(file);
        yaml_parser_delete(&parser);
        return (RETURN_FAILURE);
        }

    if(parse_config_stream_event(&parser, program_list, &event) != RETURN_SUCCESS)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The function to parse the stream event of the config file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif

        yaml_event_delete(&event);
        fclose(file);
        yaml_parser_delete(&parser);
        return (RETURN_FAILURE);
        }

    yaml_event_delete(&event);

    fclose(file);

    yaml_parser_delete(&parser);

    return (RETURN_SUCCESS);
    }

/**
* This function free the structure program_specification
*/
void free_program_specification(struct program_specification *program)
    {
    if(program == NULL)
        {
        return;
        }

    if(program->global_status.global_status_struct_init == FALSE)
        {
        return;
        }

    uint32_t cnt;

    cnt = 0;

    memset(&program->global_status, 0, sizeof(program->global_status));
    program->global_status.global_status_struct_init             = FALSE;
    program->global_status.global_status_conf_loaded             = FALSE;
    program->global_status.global_status_configuration_reloading = FALSE;
    program->global_status.global_status_need_to_restart         = FALSE;

    free(program->str_name);
    program->str_name = NULL;
    program->name_length = 0;

    free(program->str_start_command);
    program->str_start_command = NULL;

    program->number_of_process = PROGRAM_DEFAULT_NUMBER_OF_PROCESS;

    program->auto_start = PROGRAM_DEFAULT_AUTO_START;
    program->e_auto_restart = PROGRAM_DEFAULT_AUTO_RESTART;

    free(program->exit_codes);
    program->exit_codes = NULL;

    program->start_time = PROGRAM_DEFAULT_START_TIME;
    program->start_retries = PROGRAM_DEFAULT_START_RETRIES;
    program->stop_signal = PROGRAM_DEFAULT_STOP_SIGNAL;
    program->stop_time = PROGRAM_DEFAULT_STOP_TIME;
    free(program->str_stdout);
    program->str_stdout = NULL;
    free(program->str_stderr);
    program->str_stderr = NULL;

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

    free(program->str_working_directory);
    program->str_working_directory = NULL;

    program->umask = PROGRAM_DEFAULT_UMASk;
    program->e_log = PROGRAM_DEFAULT_LOG_STATUS;
    }

/**
* This function free the structure program_list
*/
void free_program_list(struct program_list *programs)
    {
    if(programs == NULL)
        {
        return;
        }

    uint32_t cnt;

    cnt = 0;

    programs->programs_loaded = FALSE;

    cnt = 0;
    while(cnt < programs->number_of_program)
        {
        free_program_specification(&(programs->programs[cnt]));
        cnt++;
        }

    free(programs->programs);
    programs->programs = NULL;
    programs->number_of_program = 0;
    }
