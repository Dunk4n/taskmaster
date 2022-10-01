#include "taskmaster.h"

uint8_t taskmaster_shell(struct program_list *program_list)
    {
    (void) program_list;
//    if(program_list == NULL)
//        return (EXIT_FAILURE);
//    if(program_list->global_status.global_status_struct_init == FALSE)
//        return (EXIT_FAILURE);
//    if(program_list->global_status.global_status_conf_loaded == FALSE)
//        return (EXIT_FAILURE);
//
////    uint8_t *line;
//    size_t  size;
//    uint8_t buffer[128];
//
//    buffer[0] = NIL;
//    size      = 0;
//
////    line = NULL;
//    while(buffer[0] != 4)
//        {
//        //write(STDIN_FILENO, SHELL_PROMPT, sizeof(SHELL_PROMPT) - 1);
//        //TODO get line
//        size = read(STDIN_FILENO, buffer, 127);
//        if(buffer[0] == 23 || buffer[0] == 5 || buffer[0] == 18 || buffer[0] == 20 || buffer[0] == 21 || buffer[0] == 27 || buffer[0] == 127)
//            {
//            size_t a = 0;
//            while(a < size)
//                {
//                printf("%u, ", buffer[a]);
//                a++;
//                }
//            printf("\n");
//            }
//        else
//            {
//            write(STDIN_FILENO, buffer, size);
//            }
//        //write(STDIN_FILENO, buffer, size);
////        if(get_next_line(0, (char **) &line) != TRUE && (line == NULL || line[0] == NIL))
////            {
////            free(line);
////            line = NULL;
////            return (EXIT_FAILURE);
////            }
////
////        ft_printf("%s\n", line);
////
////        //TODO manage command
////        free(line);
////        line = NULL;
//        }
//
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

    //TODO get term
    str_term_value = getenv("TERM");
    if(str_term_value == NULL)
        return (EXIT_FAILURE);
    if(tgetent(NULL, str_term_value) < 1)
        return (EXIT_FAILURE);

    tcgetattr(STDIN_FILENO, &(taskmaster->termios_save));
    tcgetattr(STDIN_FILENO, &(taskmaster->termios));
    taskmaster->termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &(taskmaster->termios));

    if(init_program_list(&(taskmaster->programs)) != EXIT_SUCCESS)
        return (EXIT_FAILURE);

    taskmaster->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }

void free_taskmaster(struct taskmaster *taskmaster)
    {
    if(taskmaster == NULL)
        return;
    if(taskmaster->global_status.global_status_struct_init == FALSE)
        return;

    free_program_list(&(taskmaster->programs));

    tcsetattr(STDIN_FILENO, TCSADRAIN, &(taskmaster->termios_save));

    taskmaster->programs.global_status.global_status_struct_init = FALSE;
    }

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
        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" in function "BWHT"%s"CRESET" at line "BWHT"%d"CRESET"\n    The parsing of the configuration file failed\n", __FILE__, __func__, __LINE__);
        #endif

        #ifdef DEMO
        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" at line "BWHT"%s"CRESET"\n", __FILE__, __LINE__);
        #endif

        #ifdef PRODUCTION
        fprintf(stderr, ""BRED"ERROR"CRESET"\n");
        #endif

        free_taskmaster(&taskmaster);
        return (EXIT_FAILURE);
        }

//    display_program_list(&(taskmaster.programs));
//    printf("\n\n======================== RELOAD ==============================\n\n");
//    if(reload_config_file((uint8_t *) argv[2], &(taskmaster.programs)) != EXIT_SUCCESS)
//        {
//        #ifdef DEVELOPEMENT
//        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" in function "BWHT"%s"CRESET" at line "BWHT"%d"CRESET"\n    The reload of the configuration file failed\n", __FILE__, __func__, __LINE__);
//        #endif
//
//        #ifdef DEMO
//        fprintf(stderr, ""BRED"ERROR"CRESET": in file "BWHT"%s"CRESET" at line "BWHT"%s"CRESET"\n", __FILE__, __LINE__);
//        #endif
//
//        #ifdef PRODUCTION
//        fprintf(stderr, ""BRED"ERROR"CRESET"\n");
//        #endif
//
//        free_taskmaster(&taskmaster);
//        return (EXIT_FAILURE);
//        }
//    display_program_list(&(taskmaster.programs));

    if(taskmaster_shell(&(taskmaster.programs)) != EXIT_SUCCESS)
        {
        free_taskmaster(&taskmaster);
        return (EXIT_FAILURE);
        }

    free_taskmaster(&taskmaster);

    return (EXIT_SUCCESS);
    }
