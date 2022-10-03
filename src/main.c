#include "minishell.h"
#include "taskmaster.h"

int main(int argc, char **argv) {
    struct taskmaster taskmaster;

    taskmaster.global_status.global_status_struct_init = FALSE;

    if (argc != 2) {
        printf("test with: ./taskmaster config.yaml\n");
        return (EXIT_FAILURE);
    }

    if (init_taskmaster(&taskmaster) != EXIT_SUCCESS) return (EXIT_FAILURE);

    if (parse_config_file((uint8_t *)argv[1], &(taskmaster.programs)) !=
        EXIT_SUCCESS) {
        free_taskmaster(&taskmaster);
        log_error("The parsing of the configuration file failed", __FILE__,
                  __func__, __LINE__);
    }

    if (tm_job_control(&taskmaster)) goto exit_error;

    if (taskmaster_shell(&taskmaster) != EXIT_SUCCESS) goto exit_error;

    free_taskmaster(&taskmaster);
    return (EXIT_SUCCESS);

exit_error:
    free_taskmaster(&taskmaster);
    return (EXIT_FAILURE);
}
