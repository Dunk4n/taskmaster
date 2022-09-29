#include "taskmaster.h"

int main(int argc, char **argv)
    {
    struct program_list programs;

    programs.last_program_linked_list = NULL;
    programs.number_of_program        = 0;
    programs.program_linked_list      = NULL;
    programs.programs_loaded          = FALSE;

    if(argc != 2)
        return (EXIT_FAILURE);

    if(parse_config_file((uint8_t *) argv[1], &programs) != EXIT_SUCCESS)
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

        free_program_list(&programs);
        return (EXIT_FAILURE);
        }

    display_program_list(&programs);
    free_program_list(&programs);

    return (EXIT_SUCCESS);
    }
