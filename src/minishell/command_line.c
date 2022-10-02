#include <unistd.h>
#include "minishell.h"

static int	init_term(t_command_line *command_line)
    {
    char	*term_name;

    if (!(term_name = getenv("TERM")))
        return (0);
    if (tgetent(NULL, term_name) < 1) //TODO leak...
        return (0);
    tcgetattr(STDIN_FILENO, &(command_line->termios_save));
    tcgetattr(STDIN_FILENO, &(command_line->termios));
    command_line->termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &(command_line->termios));
    return (1);
    }

void    free_command_line(t_command_line *command_line)
    {
    if(command_line == NULL)
        return ;
    if(command_line->global_status.global_status_struct_init == FALSE)
        return ;

    int	i;

    i = 0;
    while (i < HISTORY_SIZE && command_line->hist[i])
        {
        free(command_line->hist[i]);
        command_line->hist[i] = NULL;
        i++;
        }

    tcsetattr(STDIN_FILENO, TCSADRAIN, &(command_line->termios_save));

    command_line->global_status.global_status_struct_init = FALSE;
    }

uint8_t		init_command_line(t_command_line *command_line)
    {
    if(command_line == NULL)
        return (EXIT_FAILURE);
    if(command_line->global_status.global_status_struct_init == TRUE)
        return (EXIT_FAILURE);

    size_t	i;

    i = 0;
    while (i < HISTORY_SIZE)
        command_line->hist[i++] = NULL;
    ft_bzero(command_line->copy, LINE_SIZE);
    command_line->copy_end = 0;
    command_line->ret = 0;
    if (!init_term(command_line))
        return (EXIT_FAILURE);

    command_line->global_status.global_status_struct_init = TRUE;

    return (EXIT_SUCCESS);
    }
