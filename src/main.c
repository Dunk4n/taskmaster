#include "minishell.h"
#include "taskmaster.h"

void display_help(void)
    {
    printf("./taskmaster [CONFIG_FILE_PATH] [OPTION]\n");
    printf("    -d start as daemon\n");
    printf("    -c start as client\n");
    printf("\nex:\n");
    printf("    ./taskmaster config.yaml\n");
    printf("    ./taskmaster -c                 // for client\n");
    printf("    ./taskmaster config.yaml -d     // for daemon\n");
    }

int main(int argc, char **argv) {
    struct taskmaster taskmaster;

    taskmaster.global_status.global_status_struct_init = FALSE;

    if (argc < 2 || argc > 3) {
        display_help();
        return (EXIT_FAILURE);
    }

    memset(&(taskmaster.global_status), 0, sizeof(taskmaster.global_status));
    if(argc == 2 && strcmp(argv[1], "-c") == 0)
        taskmaster.global_status.global_status_start_as_client = TRUE;
    else if(argc == 3)
        {
        if(strcmp(argv[2], "-d") == 0)
            {
            taskmaster.global_status.global_status_start_as_daemon = TRUE;
            }
        else
            {
            display_help();
            return (EXIT_FAILURE);
            }
        }
    else
        {
        display_help();
        return (EXIT_FAILURE);
        }

    taskmaster.global_status.global_status_struct_init = FALSE;
    if (init_taskmaster(&taskmaster) != EXIT_SUCCESS) return (EXIT_FAILURE);

    if(taskmaster.global_status.global_status_start_as_client == FALSE)
        {
        if (parse_config_file((uint8_t *)argv[1], &(taskmaster.programs)) !=
                EXIT_SUCCESS) {
            free_taskmaster(&taskmaster);
            log_error("The parsing of the configuration file failed", __FILE__,
                    __func__, __LINE__);
        }

        //if (tm_job_control(&taskmaster.programs)) goto exit_error;
        }

    if(taskmaster.global_status.global_status_start_as_daemon == TRUE)
        {
        //TODO recv command from client
        }
    else
        {
        if (taskmaster_shell(&taskmaster) != EXIT_SUCCESS) goto exit_error;
        }

    free_taskmaster(&taskmaster);
    return (EXIT_SUCCESS);

exit_error:
    free_taskmaster(&taskmaster);
    return (EXIT_FAILURE);
}
