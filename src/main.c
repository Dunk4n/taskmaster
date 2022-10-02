#include "taskmaster.h"
#include "minishell.h"

int main(int argc, char **argv)
    {
    struct taskmaster taskmaster;

    taskmaster.global_status.global_status_struct_init = FALSE;

    if(argc != 3)
        {
        printf("test with: ./taskmaster config.yaml config_2.yaml\n");
        return (EXIT_FAILURE);
        }

    if(init_taskmaster(&taskmaster) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    if(parse_config_file((uint8_t *) argv[1], &(taskmaster.programs)) != EXIT_SUCCESS)
        {
        #ifdef DEVELOPEMENT
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The parsing of the configuration file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
        #endif

        free_taskmaster(&taskmaster);
        return (EXIT_FAILURE);
        }

//    display_program_list(&(taskmaster.programs));
//    printf("\n\n======================== RELOAD ==============================\n\n");
//    if(reload_config_file((uint8_t *) argv[2], &(taskmaster.programs)) != EXIT_SUCCESS)
//        {
//        #ifdef DEVELOPEMENT
//        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m in function \033[1m%s\033[0m at line \033[1m%d\033[0m\n    The reload of the configuration file failed\n", __FILE__, __func__, __LINE__);
//        #endif
//
//        #ifdef DEMO
//        fprintf(stderr, "\033[1;31mERROR\033[0m: in file \033[1m%s\033[0m at line \033[1m%s\033[0m\n", __FILE__, __LINE__);
//        #endif
//
//        #ifdef PRODUCTION
//        fprintf(stderr, "\033[1;31mERROR\033[0m\n");
//        #endif
//
//        free_taskmaster(&taskmaster);
//        return (EXIT_FAILURE);
//        }
//    display_program_list(&(taskmaster.programs));

    //if(taskmaster_shell(&taskmaster) != EXIT_SUCCESS)
    //    {
    //    free_taskmaster(&taskmaster);
    //    return (EXIT_FAILURE);
    //    }

    if(taskmaster_shell(&taskmaster) != EXIT_SUCCESS)
        {
        free_taskmaster(&taskmaster);
        return (EXIT_FAILURE);
        }

    free_taskmaster(&taskmaster);

    return (EXIT_SUCCESS);
    }
