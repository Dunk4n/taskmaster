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
static uint8_t set_program_default_value(struct program_specification *program)
    {
    if(program == NULL)
        {
        return (EXIT_FAILURE);
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
    program->next_program = NULL;

    program->exit_codes_number = 0;

    program->exit_codes = malloc(1 * sizeof(uint8_t));
    if(program->exit_codes == NULL)
        return (EXIT_FAILURE);
    program->exit_codes[0] = PROGRAM_DEFAULT_EXIT_CODE;
    program->exit_codes_number = 1;

    program->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }

/**
* This function reset to the default value the structure program_specification without the name
*/
static uint8_t reset_program_default_value_without_name(struct program_specification *program)
    {
    if(program == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program->global_status.global_status_struct_init == FALSE)
        {
        return (EXIT_FAILURE);
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
        return (EXIT_FAILURE);
    program->exit_codes[0] = PROGRAM_DEFAULT_EXIT_CODE;
    program->exit_codes_number = 1;

    program->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }

/**
* This function add a new structure program_specification in the dynamic array of structure program_specification in the structure program_list
*/
static uint8_t add_new_program(struct program_list *program_list, uint8_t *program_name, size_t program_name_length)
    {
    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_name == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_name_length == 0 || program_name_length == SIZE_MAX)
        {
        return (EXIT_FAILURE);
        }

    if(program_list->number_of_program == UINT32_MAX)
        {
        return (EXIT_FAILURE);
        }

    if(program_list->program_linked_list != NULL && program_list->last_program_linked_list == NULL)
        return (EXIT_FAILURE);

    if(program_list->program_linked_list == NULL && program_list->last_program_linked_list != NULL)
        return (EXIT_FAILURE);

    struct program_specification *program_tmp;
    size_t                        cnt;

    program_tmp = NULL;
    cnt         = 0;

    program_tmp = malloc(sizeof(struct program_specification));
    if(program_tmp == NULL)
        return (EXIT_FAILURE);

    if(set_program_default_value(program_tmp) != EXIT_SUCCESS)
        {
        free(program_tmp);
        program_tmp = NULL;
        return (EXIT_FAILURE);
        }

    if(program_list->program_linked_list == NULL)
        {
        program_list->program_linked_list      = program_tmp;
        program_list->last_program_linked_list = program_tmp;

        program_list->number_of_program = 1;
        }
    else
        {
        program_list->last_program_linked_list->next_program = program_tmp;
        program_list->last_program_linked_list = program_tmp;

        program_list->number_of_program++;
        }

    program_list->last_program_linked_list->str_name = malloc((program_name_length + 1) * sizeof(uint8_t));
    if(program_list->last_program_linked_list->str_name == NULL)
        return (EXIT_FAILURE);

    cnt = 0;
    while(cnt < program_name_length)
        {
        program_list->last_program_linked_list->str_name[cnt] = program_name[cnt];

        cnt++;
        }

    program_list->last_program_linked_list->str_name[cnt] = NIL;
    program_list->last_program_linked_list->name_length = cnt;

    return (EXIT_SUCCESS);
    }

/**
* This function check if all the required attribut of the structure program_specification has been set
*/
static uint8_t check_required_attribute_in_program(struct program_specification *program)
    {
    if(program == NULL)
        return (EXIT_FAILURE);

    if(program->str_name == NULL || program->name_length == 0)
        return (EXIT_FAILURE);
    if(program->str_start_command == NULL)
        return (EXIT_FAILURE);
    if(program->exit_codes == NULL || program->exit_codes_number == 0)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

/**
* This function parse all the attribute of one program in the YAML config file
*/
static uint8_t parse_config_program_attribute(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    uint8_t cnt;

    cnt = 0;

    if(event->type != YAML_MAPPING_START_EVENT)
        return (EXIT_FAILURE);

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
            return (EXIT_FAILURE);
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
                    if(g_program_specification_field_load_function[cnt](parser, program, event) != EXIT_SUCCESS)
                        return (EXIT_FAILURE);

                    break;
                    }

                cnt++;
                }
            }
        else
            return (EXIT_FAILURE);
        }

    if(check_required_attribute_in_program(program) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

/**
* This function parse one program in the YAML config file
*/
static uint8_t parse_config_one_program(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    struct program_specification *actual_program_specification;
    uint32_t                      cnt;

    actual_program_specification = NULL;
    cnt                          = 0;

    if(event->type != YAML_SCALAR_EVENT)
        return (EXIT_FAILURE);

    if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
        return (EXIT_FAILURE);

    if(program_list->program_linked_list != NULL && program_list->number_of_program > 0)
        {
        actual_program_specification = program_list->program_linked_list;
        cnt = 0;
        while(cnt < program_list->number_of_program && actual_program_specification != NULL)
            {
            if(actual_program_specification->global_status.global_status_struct_init == TRUE && actual_program_specification->name_length != 0 && actual_program_specification->str_name != NULL && strcmp((char *) event->data.scalar.value, (char *) actual_program_specification->str_name) == 0)
                {
                if(reset_program_default_value_without_name(actual_program_specification) != EXIT_SUCCESS)
                    return (EXIT_FAILURE);
                actual_program_specification->global_status.global_status_configuration_reloading = TRUE;
                break;
                }

            actual_program_specification = actual_program_specification->next_program;
            cnt++;
            }

        if(cnt >= program_list->number_of_program)
            actual_program_specification = NULL;
        }

    if(actual_program_specification == NULL)
        {
        if(add_new_program(program_list, event->data.scalar.value, event->data.scalar.length) != EXIT_SUCCESS)
            return (EXIT_FAILURE);

        if(program_list->last_program_linked_list == NULL)
            return (EXIT_FAILURE);

        actual_program_specification = program_list->last_program_linked_list;
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
        return (EXIT_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
        return (EXIT_FAILURE);

    if(parse_config_program_attribute(parser, actual_program_specification, event) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(event->type != YAML_MAPPING_END_EVENT)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

/**
* This function parse all the programs in the YAML config file
*/
static uint8_t parse_config_all_programs(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
        return (EXIT_FAILURE);

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
            return (EXIT_FAILURE);
            }

        switch(event->type)
            {
            case(YAML_MAPPING_END_EVENT):
                return (EXIT_SUCCESS);
            case(YAML_SCALAR_EVENT):
                if(event->data.scalar.value == NULL || event->data.scalar.length == 0)
                    return (EXIT_FAILURE);
                if(parse_config_one_program(parser, program_list, event) != EXIT_SUCCESS)
                    return (EXIT_FAILURE);
                break;
            default:
                return (EXIT_FAILURE);
            }
        }

    return (EXIT_SUCCESS);
    }

/**
* This function parse the mapping event of the name programs in the YAML config file
*/
static uint8_t parse_config_mapping_event_programs(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event->type != YAML_MAPPING_START_EVENT)
        return (EXIT_FAILURE);

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
        return (EXIT_FAILURE);
        }

    switch(event->type)
        {
        case(YAML_MAPPING_END_EVENT):
            return (EXIT_SUCCESS);
        case(YAML_SCALAR_EVENT):
            if(event->data.scalar.value == NULL || event->data.scalar.length == 0 || strcmp((char *) event->data.scalar.value, PROGRAM_CATEGORY_STR) != 0)
                return (EXIT_FAILURE);

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
                return (EXIT_FAILURE);
                }

            switch(event->type)
                {
                case(YAML_MAPPING_END_EVENT):
                    return (EXIT_SUCCESS);
                case(YAML_MAPPING_START_EVENT):
                    if(parse_config_all_programs(parser, program_list, event) != EXIT_SUCCESS)
                        return (EXIT_FAILURE);
                    break;
                default:
                    return (EXIT_FAILURE);
                }
            break;
        default:
            return (EXIT_FAILURE);
        }

    return (EXIT_SUCCESS);
    }

/**
* This function parse the document event in the YAML config file
*/
static uint8_t parse_config_document_event(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event->type != YAML_DOCUMENT_START_EVENT)
        return (EXIT_FAILURE);

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
            return (EXIT_FAILURE);
            }

        switch(event->type)
            {
            case(YAML_DOCUMENT_END_EVENT):
                return (EXIT_SUCCESS);
            case(YAML_MAPPING_START_EVENT):
                if(parse_config_mapping_event_programs(parser, program_list, event) != EXIT_SUCCESS)
                    return (EXIT_FAILURE);
                break;
            default:
                return (EXIT_FAILURE);
            }
        }

    return (EXIT_SUCCESS);
    }

/**
* This function parse the stream event in the YAML config file
*/
static uint8_t parse_config_stream_event(yaml_parser_t *parser, struct program_list *program_list, yaml_event_t *event)
    {
    if(parser == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(event->type != YAML_STREAM_START_EVENT)
        return (EXIT_FAILURE);

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
        return (EXIT_FAILURE);
        }

    if(event->type == YAML_STREAM_END_EVENT)
        return (EXIT_SUCCESS);

    if(event->type == YAML_DOCUMENT_START_EVENT)
        {
        if(parse_config_document_event(parser, program_list, event) != EXIT_SUCCESS)
            return (EXIT_FAILURE);
        }
    else
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
    }

/**
* This function parse the YAML config file and store the content in the structure program_list
*/
uint8_t parse_config_file(uint8_t *file_name, struct program_list *program_list)
    {
    if(file_name == NULL)
        {
        return (EXIT_FAILURE);
        }

    if(program_list == NULL)
        {
        return (EXIT_FAILURE);
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
        return (EXIT_FAILURE);
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
        return (EXIT_FAILURE);
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
        return (EXIT_FAILURE);
        }

    if(event.type != YAML_STREAM_START_EVENT)
        {
        yaml_event_delete(&event);
        fclose(file);
        yaml_parser_delete(&parser);
        return (EXIT_FAILURE);
        }

    if(parse_config_stream_event(&parser, program_list, &event) != EXIT_SUCCESS)
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
        return (EXIT_FAILURE);
        }

    yaml_event_delete(&event);

    fclose(file);

    yaml_parser_delete(&parser);

    return (EXIT_SUCCESS);
    }

/**
* This function free the structure program_specification
*/
static void free_program_specification(struct program_specification *program)
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

    program->next_program = NULL;
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

    struct program_specification *actual_program;
    uint32_t                      cnt;

    actual_program = NULL;
    cnt            = 0;

    programs->programs_loaded = FALSE;

    cnt = 0;
    while(cnt < programs->number_of_program && programs->program_linked_list != NULL)
        {
        actual_program = programs->program_linked_list;

        programs->program_linked_list = actual_program->next_program;

        free_program_specification(actual_program);
        free(actual_program);
        cnt++;
        }

    free(programs->program_linked_list);
    programs->program_linked_list = NULL;
    programs->last_program_linked_list = NULL;
    programs->number_of_program = 0;
    }

/**
* This function display the structure program_specification
*/
static void display_program_specification(struct program_specification *program)
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

    if(program->global_status.global_status_struct_init == TRUE)
        printf("The structure program specification is INIT\n");
    else
        printf("The structure program specification is NOT INIT\n");

    if(program->global_status.global_status_conf_loaded == TRUE)
        printf("The structure is LOADED\n");
    else
        printf("The structure is NOT LOADED\n");

    if(program->global_status.global_status_configuration_reloading == TRUE)
        printf("The structure is RELOADING\n");
    else
        printf("The structure is NOT RELOADING\n");

    if(program->global_status.global_status_need_to_restart == TRUE)
        printf("The structure need to RESTART\n");
    else
        printf("The structure does NOT need to RESTART\n");

    printf("\n");

    printf("NAME: [%s]\n", program->str_name);
    printf("START_COMMAND: [%s]\n", program->str_start_command);
    printf("NUMBER_OF_PROCESS: %u\n", program->number_of_process);
    if(program->auto_start == TRUE)
        printf("AUTO_START: TRUE\n");
    else
        printf("AUTO_START: FALSE\n");

    switch(program->e_auto_restart)
        {
        case(PROGRAM_AUTO_RESTART_FALSE):
            printf("AUTO_RESTART: FALSE\n");
            break;
        case(PROGRAM_AUTO_RESTART_TRUE):
            printf("AUTO_RESTART: TRUE\n");
            break;
        case(PROGRAM_AUTO_RESTART_UNEXPECTED):
            printf("AUTO_RESTART: UNEXPECTED\n");
            break;
        default:
            break;
        };

    if(program->exit_codes != NULL)
        {
        printf("EXIT_CODE_NUMBER: %u\n", program->exit_codes_number);
        printf("EXIT_CODE:");
        cnt = 0;
        while(cnt < program->exit_codes_number)
            {
            printf(" %u", program->exit_codes[cnt]);

            cnt++;
            }
        printf("\n");
        }

    printf("START_TIME: %u\n", program->start_time);
    printf("START_RETRIES: %u\n", program->start_retries);
    printf("STOP_SIGNAL: %u\n", program->stop_signal);
    printf("STOP_TIME: %u\n", program->stop_time);

    printf("STDOUT: [%s]\n", program->str_stdout);
    printf("STDERR: [%s]\n", program->str_stderr);

    if(program->env != NULL)
        {
        printf("ENV:\n");
        cnt = 0;
        while(cnt < program->env_length)
            {
            if(program->env[cnt] == NULL)
                printf("%u\tNULL\n", cnt);
            else
                printf("%u\t%s\n", cnt, program->env[cnt]);

            cnt++;
            }
        }

    printf("WORKING_DIRECTORY: [%s]\n", program->str_working_directory);
    printf("UMASK: %c%c%c\n", ((program->umask >> 6) & 7) + '0', ((program->umask >> 3) & 7) + '0', (program->umask & 7) + '0');

    switch(program->e_log)
        {
        case(PROGRAM_LOG_FALSE):
            printf("LOG: FALSE\n");
            break;
        case(PROGRAM_LOG_TRUE):
            printf("LOG: TRUE\n");
            break;
        case(PROGRAM_LOG_MAIL):
            printf("LOG: UNEXPECTED\n");
            break;
        default:
            break;
        };

    printf("\n");
    if(program->next_program == NULL)
        printf("NEXT_PROGRAM: NULL\n");
    else
        {
        if(program->next_program->str_name != NULL)
            printf("NEXT_PROGRAM: %p, [%s]\n", program->next_program, program->next_program->str_name);
        else
            printf("NEXT_PROGRAM: %p\n", program->next_program);
        }
    }

/**
* This function display the structure program_list
*/
void display_program_list(struct program_list *programs)
    {
    if(programs == NULL)
        {
        return;
        }

    struct program_specification *actual_program;
    uint32_t                      cnt;

    actual_program = NULL;
    cnt            = 0;

    printf("\033[1;92mPROGRAM SPECIFICATION LINKED LIST\033[0m:\n");
    actual_program = programs->program_linked_list;
    cnt = 0;
    while(cnt < programs->number_of_program && actual_program != NULL)
        {
        printf("\n\033[1;93mPROGRAM SPECIFICATION %u\033[0m:\n\n", cnt);
        display_program_specification(actual_program);

        actual_program = actual_program->next_program;
        cnt++;
        }

    printf("\n\033[1;92mPROGRAM SPECIFICATION LINKED LIST END\033[0m:\n");
    }
