#include "taskmaster.h"
#include "minishell.h"

int main(int argc, char **argv)
    {
    struct taskmaster taskmaster;

    taskmaster.global_status.global_status_struct_init = FALSE;

    if(argc != 2)
        {
        printf("test with: ./taskmaster config.yaml\n");
        return (EXIT_FAILURE);
        }

    if(init_taskmaster(&taskmaster) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(parse_config_file((uint8_t *) argv[1], &(taskmaster.programs)) != EXIT_SUCCESS)
    {
        free_taskmaster(&taskmaster);
        log_error("The parsing of the configuration file failed",
                __FILE__, __func__, __LINE__);
    }

    if (tm_server(&taskmaster)) {
        free_taskmaster(&taskmaster);
        log_error("server failed",
                __FILE__, __func__, __LINE__);
    }

    if(taskmaster_shell(&taskmaster) != EXIT_SUCCESS)
        {
        free_taskmaster(&taskmaster);
        return (EXIT_FAILURE);
        }

    free_taskmaster(&taskmaster);

    return (EXIT_SUCCESS);
    }
