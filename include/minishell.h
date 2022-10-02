/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/15 19:07:52 by niduches          #+#    #+#             */
/*   Updated: 2020/01/14 10:43:15 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_BONUS_H
# define MINISHELL_BONUS_H

# include <curses.h>
# include <term.h>
# include "../libft/libft.h"

# define LINE_SIZE 4096
# define HISTORY_SIZE 500
# define SHELL_EXIT_MESSAGE ("exit\n")
# define SHELL_MAX_ARGUMENT ((uint8_t) 10)

typedef struct s_command_line
{
    struct
    {
        // FIRST_BIT     Structure is init          1 = Y / 0 = N
        uint8_t global_status_struct_init : 1;
    } global_status;

    char   ret;
    struct termios termios;
    struct termios termios_save;
    char   *hist[HISTORY_SIZE];
    char   tmp[LINE_SIZE];
    char   copy[LINE_SIZE];
    int    copy_end;
    int    idx;
}    t_command_line;

typedef struct s_cursor
{
    int  startx;
    int  starty;
    int  x;
    int  y;
    int  idx;
    int  col;
    int  line;
    int  line_max;
    int  line_size;
    int  term_col;
    int  term_line;
    t_command_line *command_line;
}    t_cursor;

uint8_t   init_command_line(t_command_line *command_line);
int       ft_exit(size_t ac, char **av, t_command_line *command_line);
int       ft_echo(size_t ac, char **av, t_command_line *command_line);
char    **get_command_line(t_command_line *command_line, char *to_find);
int       ft_command_line(size_t ac, char **av, t_command_line *command_line);
int       ft_pwd(size_t ac, char **av, t_command_line *command_line);
void      get_all_instruction(char *line, t_command_line *command_line);
int       ft_export(size_t ac, char **av, t_command_line *command_line);
int       ft_unset(size_t ac, char **av, t_command_line *command_line);
int       in_str(char c, char const *charset);
char    **custom_split_arg(char const *str);
char    **custom_split_instr(char const *str);
char    **custom_split_sep(char *line);
int       get_nb_sep(char *line);
int       is_only_space(char *line, size_t end);
size_t    pass_normal(char *line);
size_t    is_sep(char *line);
void      get_exec_path(char *path, char *name, t_command_line *command_line);
char    **custom_split_sep_pipe(char *line);
int       get_nb_sep_pipe(char *line);
size_t    pass_normal_pipe(char *line);
size_t    is_sep_pipe(char *line);
int       ft_cd(size_t ac, char **av, t_command_line *command_line);
void      free_command_line(t_command_line *command_line);
int       good_logic_syntax(char **inst);

void      get_cursor_position(int *col, int *row);
void      move_cursor(int col, int row);
void      cursor_exec(char *cmd);
void      cursor_exec_at(char *cmd, int x, int y);

int       get_edit_line(t_command_line *command_line, char **line);
uint8_t   add_in_history(t_command_line *command_line, char *line);
void      add_char_in_line(char *line, char *buff, t_cursor *cur);
void      init_cursor(char *line, t_cursor *cur, t_command_line *command_line);
void      update_cursor_pos(t_cursor *cur, char *line);
int       is_term_command(char *buff, t_cursor *cur, char *line);
int       make_term_command(char *line, char *buff, t_cursor *cur, t_command_line *command_line);
void      charge_from_history(char *line, char *src, t_cursor *cur);
void      add_all_line(char *line, t_cursor *cur);

#endif
