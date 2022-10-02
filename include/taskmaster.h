#ifndef TASKMASTER_H
# define TASKMASTER_H

# include <fcntl.h>
# include <inttypes.h>
# include <limits.h>
# include <pthread.h>
# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <term.h>
# include "yaml.h"
# include "minishell.h"

# ifndef FALSE
#  define FALSE (0)
# endif

# ifndef TRUE
#  define TRUE (1)
# endif

# include "ANSI_color_codes.h"

# define NIL            ('\0')

# define FIRST_BIT      (0x01)
# define SECOND_BIT     (0x02)
# define THIRD_BIT      (0x04)
# define FOURTH_BIT     (0x08)
# define FIFTH_BIT      (0x10)
# define SIXTH_BIT      (0x20)
# define SEVENTH_BIT    (0x40)
# define EIGHTH_BIT     (0x80)

# define NOT_FIRST_BIT      (0xFE)
# define NOT_SECOND_BIT     (0xFD)
# define NOT_THIRD_BIT      (0xFB)
# define NOT_FOURTH_BIT     (0xF7)
# define NOT_FIFTH_BIT      (0xEF)
# define NOT_SIXTH_BIT      (0xDF)
# define NOT_SEVENTH_BIT    (0xBF)
# define NOT_EIGHTH_BIT     (0x7F)

/**
* This enumeration represente all the posible value for the attribute autorestart in the structure program_specification
*/
enum program_auto_restart_status
{
    PROGRAM_AUTO_RESTART_FALSE = 0,
    PROGRAM_AUTO_RESTART_TRUE,
    PROGRAM_AUTO_RESTART_UNEXPECTED,
    NUMBER_OF_PROGRAM_AUTO_RESTART_STATUS
};

# define PROGRAM_CATEGORY_STR "programs"
# define PROGRAM_DEFAULT_NUMBER_OF_PROCESS (1)
# define PROGRAM_DEFAULT_AUTO_START (TRUE)
# define PROGRAM_DEFAULT_AUTO_RESTART (PROGRAM_AUTO_RESTART_UNEXPECTED)
# define PROGRAM_DEFAULT_START_TIME (0)
# define PROGRAM_DEFAULT_START_RETRIES (3)
# define PROGRAM_DEFAULT_STOP_SIGNAL (SIGTERM)
# define PROGRAM_DEFAULT_STOP_TIME (10)
# define PROGRAM_DEFAULT_LOG_STATUS (PROGRAM_LOG_TRUE)
# define PROGRAM_DEFAULT_UMASk (S_IWOTH | S_IWGRP)
# define PROGRAM_DEFAULT_EXIT_CODE (0)

# define SHELL_PROMPT ("$> ")

/**
* This enumeration represente all the posible attribute in the structure program_specification
*/
enum program_specification_field
    {
    PROGRAM_CMD = 0,
    PROGRAM_NUMPROCS,
    PROGRAM_AUTOSTART,
    PROGRAM_AUTORESTART,
    PROGRAM_EXITCODES,
    PROGRAM_STARTTIME,
    PROGRAM_STARTRETRIES,
    PROGRAM_STOPSIGNAL,
    PROGRAM_STOPTIME,
    PROGRAM_STDOUT,
    PROGRAM_STDERR,
    PROGRAM_ENV,
    PROGRAM_WORKINGDIR,
    PROGRAM_UMASK,
    PROGRAM_LOG,
    NUMBER_OF_PROGRAM_SPECIFICATION_FIELD
    };

/**
* This enumeration represente all the posible value for the attribute log in the structure program_specification
*/
enum program_log_status
{
    PROGRAM_LOG_FALSE = 0,
    PROGRAM_LOG_TRUE,
    PROGRAM_LOG_MAIL,
    NUMBER_OF_PROGRAM_LOG_STATUS
};

/**
* This structure represente a program that will be executed by taskmaster
*/
struct program_specification
{
    struct
        {
        // FIRST_BIT     Structure is init          1 = Y / 0 = N
        uint8_t global_status_struct_init             : 1;

        // SECOND_BIT     Configuration is loaded          1 = Y / 0 = N
        uint8_t global_status_conf_loaded             : 1;

        // THIRD_BIT    Configuration is reloading 1 = Y / 0 = N
        uint8_t global_status_configuration_reloading : 1;

        // FOURTH_BIT     Need to restart            1 = Y / 0 = N
        uint8_t global_status_need_to_restart         : 1;

        //                Need to stop               1 = Y / 0 = N
        uint8_t global_status_need_to_stop            : 1;

        //                Need to start              1 = Y / 0 = N
        uint8_t global_status_need_to_start           : 1;

        //                Need to remove             1 = Y / 0 = N
        uint8_t global_status_need_to_remove          : 1;

        //                Started                    1 = Y / 0 = N
        uint8_t global_status_started                 : 1;
        } global_status;

    uint8_t                          *str_name;
    size_t                            name_length;
    uint8_t                          *str_start_command;        // 1.  The command to use to launch the program
    uint32_t                          number_of_process;        // 2.  The number of processes to start and keep running
    uint8_t                           auto_start;               // 3.  Whether to start this program at launch or not
    enum program_auto_restart_status  e_auto_restart;           // 4.  Whether the program should be restarted always, never, or on unexpected exits only
    uint8_t                          *exit_codes;               // 5.  Which return codes represent an "expected" exit status // array of unsigned int 8 of error code
    uint16_t                          exit_codes_number;
    uint32_t                          start_time;               // 6.  How long the program should be running after it’s started for it to be considered "successfully started"
    uint32_t                          start_retries;            // 7.  How many times a restart should be attempted before aborting
    int32_t                           stop_signal;              // 8.  Which signal should be used to stop (i.e. exit gracefully) the program
    uint32_t                          stop_time;                // 9.  How long to wait after a graceful stop before killing the program
    char                             *str_stdout;               // 10. Options to discard the program’s stdout/stderr or to redirect them to files
    char                             *str_stderr;
    uint8_t                         **env;                      // 11. Environment variables to set before launching the program // array of strings
    uint32_t                          env_length;
    uint8_t                          *str_working_directory;    // 12. A working directory to set before launching the program
    mode_t                            umask;                    // 13. An umask to set before launching the program
    enum program_log_status           e_log;                    // 3.  More advanced logging/reporting facilities (Alerts via email/http/syslog/etc...)

    struct program_specification *restart_tmp_program;

    struct log {
        int32_t out; /* fd log stdout */
        int32_t err; /* fd log stderr */
    } log;

    /* runtime data relative to a thread. */
    struct thread_data {
      uint32_t rid;  /* rank id of current thread/proc. Index for an array */
      pthread_t tid; /* thread id of current thread */
      uint32_t pid;  /* pid of current process */
      /* how many time the process can be restarted */
      uint16_t restart_counter;
      uint8_t exit_status; /* value of exit from the current process */
    } *thrd; /* array of thread_data. One thread per processus */

    /* Linked list */
    struct program_specification *next;
};

/**
* This structure represente the list of all the program that will be executed by taskmaster
*/
struct program_list
{
    struct
        {
        // FIRST_BIT     Structure is init          1 = Y / 0 = N
        uint8_t global_status_struct_init             : 1;

        // SECOND_BIT     Configuration is loaded   1 = Y / 0 = N
        uint8_t global_status_conf_loaded             : 1;
        } global_status;

    pthread_mutex_t               mutex_program_linked_list;
    struct program_specification *program_linked_list;
    struct program_specification *last_program_linked_list;
    uint32_t                      number_of_program;
};

struct taskmaster
{
    /* taskmaster status . Y = 0, N = 1 */
    struct {
        uint8_t global_status_struct_init      : 1; /* structure is init */
        uint8_t global_status_exit             : 1; /* exit the program */
    } global_status;

    /* node with head, tail & data of program list */
    struct program_list programs;

    t_command_line command_line;

    struct termios  termios;
    struct termios  termios_save;

    uint8_t         str_line[4096];
    uint16_t        line_size;
    uint16_t        pos_in_line;
    int32_t         cursor_pos_x;
    int32_t         cursor_pos_y;
};

/**
* This enumeration represente all the posible shell command
*/
enum shell_command
{
    SHELL_COMMAND_STATUS,
    SHELL_COMMAND_START,
    SHELL_COMMAND_STOP,
    SHELL_COMMAND_RESTART,
    SHELL_COMMAND_RELOAD_CONF,
    SHELL_COMMAND_EXIT,
    NUMBER_OF_SHELL_COMMAND
};

/* configuration_parsing_program_field_load_function.c */
uint8_t program_field_cmd_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_numprocs_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_autostart_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_autorestart_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_exitcodes_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_starttime_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_startretries_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_stopsignal_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_stoptime_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_stdout_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_stderr_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_env_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_workingdir_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_umask_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);
uint8_t program_field_log_load_function(yaml_parser_t *parser, struct program_specification *program, yaml_event_t *event);

/* configuration_parsing.c */
uint8_t init_program_list(struct program_list *program_list);
uint8_t parse_config_file(uint8_t *file_name, struct program_list *program_list);
uint8_t reload_config_file(uint8_t *file_name, struct program_list *program_list);
void display_program_specification(struct program_specification *program);
void display_program_list(struct program_list *programs);
void free_linked_list_in_program_list(struct program_list *programs);
void free_program_list(struct program_list *programs);
void free_program_specification(struct program_specification *program);

/* error.c */
#define log_error(msg, file, func, line) \
  do {                                   \
    err_display(msg, file, func, line);  \
    return (EXIT_FAILURE);               \
  } while (0)

void err_display(const char *msg, const char *file, const char *func,
			uint32_t line);

/* taskmaster.c */
uint8_t taskmaster_shell(struct taskmaster *taskmaster);
uint8_t init_taskmaster(struct taskmaster *taskmaster);
void    free_taskmaster(struct taskmaster *taskmaster);

/* server.c */
#define UNINITIALIZED_FD (-42)
#define FD_ERR (-1)
uint8_t tm_server(struct taskmaster *taskmaster);

/* execute_command_line.c */
uint8_t *get_next_instruction(char *line, int32_t *id);
uint8_t  separate_line_in_instruction(char *line, uint8_t **instructions);
uint8_t  execute_command_line(struct taskmaster *taskmaster, char *line);

/* shell_command.c */
uint8_t  shell_command_status_function(struct taskmaster *taskmaster, uint8_t **arguments);
uint8_t  shell_command_start_function(struct taskmaster *taskmaster, uint8_t **arguments);
uint8_t  shell_command_stop_function(struct taskmaster *taskmaster, uint8_t **arguments);
uint8_t  shell_command_restart_function(struct taskmaster *taskmaster, uint8_t **arguments);
uint8_t  shell_command_reload_conf_function(struct taskmaster *taskmaster, uint8_t **arguments);
uint8_t  shell_command_exit_function(struct taskmaster *taskmaster, uint8_t **arguments);

#endif
