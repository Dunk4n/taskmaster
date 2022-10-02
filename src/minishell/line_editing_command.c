#include <unistd.h>
#include <stdio.h>
#include "minishell.h"

static void move_one_char(t_cursor *cur, int dir)
    {
    cur->col += dir;
    cur->idx += dir;
    }

static void move_up(char *line, char *buff, t_cursor *cur, int *idx)
    {
    int    i;

    if (*buff == 27 && !ft_strncmp(buff + 1, "[1;5A", 5))
        {
        if (cur->line > 0)
            {
            cur->idx -= cur->col;
            if (cur->idx > 0)
                cur->idx--;
            while (cur->idx - 1 == 0 || (cur->idx - 1 > 0 &&
                        line[cur->idx - 1]))
                cur->idx--;
            i = 0;
            while (i < cur->col && line[cur->idx + i])
                i++;
            cur->idx += i;
            cur->col = i;
            cur->line--;
            }
        (*idx) += 5;
        }
    }

static void move_down(char *line, char *buff, t_cursor *cur, int *idx)
    {
    int    i;

    if (*buff == 27 && !ft_strncmp(buff + 1, "[1;5B", 5))
        {
        if (cur->line < cur->line_max - 1)
            {
            i = 0;
            while (cur->idx < cur->line_size && line[cur->idx])
                cur->idx++;
            if (cur->idx < cur->line_size)
                cur->idx++;
            while (i < cur->col && line[cur->idx + i])
                i++;
            cur->idx += i;
            cur->col = i;
            cur->line++;
            }
        (*idx) += 5;
        }
    }

static int  move_cur(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    if (*buff == 27 && !ft_strncmp(buff + 1, "[D", 2) && cur->idx > 0 &&
            line[cur->idx - 1])
        move_one_char(cur, -1);
    if (*buff == 27 && !ft_strncmp(buff + 1, "[C", 2) && line[cur->idx] != '\0')
        move_one_char(cur, 1);
    if (*buff == 27 && !ft_strncmp(buff + 1, "[B", 2) && command_line->idx >= 0)
        {
        command_line->idx--;
        if(command_line->idx >= 0)
            charge_from_history(line, command_line->hist[command_line->idx], cur);
        else
            charge_from_history(line, command_line->tmp, cur);
        }
    if (*buff == 27 && !ft_strncmp(buff + 1, "[H", 2))
        while (cur->idx > 0 && cur->col > 0 && line[cur->idx - 1])
            move_one_char(cur, -1);
    if (*buff == 27 && !ft_strncmp(buff + 1, "[F", 2))
        while (line[cur->idx])
            move_one_char(cur, 1);
    if (*buff == 27 && (!ft_strncmp(buff + 1, "[D", 2) || !ft_strncmp(buff + 1,
                    "[C", 2) || !ft_strncmp(buff + 1, "[B", 2) || !ft_strncmp(buff + 1, "[H", 2) ||
                !ft_strncmp(buff + 1, "[F", 2)))
        return (2);
    return (0);
    }

static int  move_history(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    int    i;

    if (*buff == 27 && !ft_strncmp(buff + 1, "[A", 2))
        {
        if (command_line->idx < HISTORY_SIZE && command_line->hist[command_line->idx + 1] != NULL)
            {
            i = -1;
            while (command_line->idx == -1 && ++i < cur->line_size)
                command_line->tmp[i] = (!line[i]) ? '\n' : line[i];
            command_line->idx++;
            charge_from_history(line, command_line->hist[command_line->idx], cur);
            }
        return (2);
        }
    if (*buff == 27 && !ft_strncmp(buff + 1, "[1;5D", 5))
        {
        while (cur->idx > 0 && cur->col > 0 && line[cur->idx - 1] &&
                !ft_isalnum(line[cur->idx - 1]))
            move_one_char(cur, -1);
        while (cur->idx > 0 && cur->col > 0 && ft_isalnum(line[cur->idx - 1]))
            move_one_char(cur, -1);
        return (5);
        }
    return (0);
    }

static int  term_past(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    int    i;

    if (*buff == 18)
        {
        i = 0;
        while (i < command_line->copy_end && cur->idx < LINE_SIZE - 1)
            {
            if (!command_line->copy[i])
                {
                add_char_in_line(line, "\n", cur);
                if (cur->y == cur->term_line - 1)
                    cur->starty--;
                write(1, "\n", 1);
                cur->col = -1;
                cur->line++;
                cur->line_max++;
                }
            else
                add_char_in_line(line, command_line->copy + i, cur);
            i++;
            }
        }
    return (0);
    }

static int  term_cut(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    if (*buff == 21)
        {
        ft_memcpy(command_line->copy, line, LINE_SIZE);
        command_line->copy_end = cur->line_size;
        ft_bzero(line, LINE_SIZE);
        cur->x = cur->startx;
        cur->y = cur->starty;
        cur->idx = 0;
        cur->col = 0;
        cur->line = 0;
        cur->line_max = 1;
        cur->line_size = 0;
        }
    else if (*buff == 5)
        {
        ft_bzero(command_line->copy, LINE_SIZE);
        ft_memcpy(command_line->copy, line + cur->idx, LINE_SIZE - cur->idx);
        ft_bzero(line + cur->idx, LINE_SIZE - cur->idx);
        command_line->copy_end = cur->line_size - cur->idx;
        cur->line_max = cur->line + 1;
        cur->line_size = cur->idx;
        }
    return (0);
    }

static int  term_copy(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    int    tmp;

    if (*buff == 20)
        {
        ft_memcpy(command_line->copy, line, LINE_SIZE);
        command_line->copy_end = cur->line_size;
        return (0);
        }
    if (*buff == 23)
        {
        ft_bzero(command_line->copy, LINE_SIZE);
        ft_memcpy(command_line->copy, line + cur->idx, LINE_SIZE - cur->idx);
        command_line->copy_end = cur->line_size - cur->idx;
        return (0);
        }
    tmp = 0;
    move_up(line, buff, cur, &tmp);
    move_down(line, buff, cur, &tmp);
    tmp += term_cut(line, buff, cur, command_line);
    tmp += term_past(line, buff, cur, command_line);
    tmp += move_cur(line, buff, cur, command_line);
    tmp += move_history(line, buff, cur, command_line);
    return (tmp);
    }

int         make_term_command(char *line, char *buff, t_cursor *cur, t_command_line *command_line)
    {
    int    idx;

    idx = 1;
    if (*buff == '\n' || (*buff == 4 && !cur->line_size))
        return (0);
    if (*buff == 127 && cur->idx > 0 && line[cur->idx - 1])
        {
        ft_memmove(line + cur->idx - 1, line + cur->idx,
                cur->line_size - cur->idx + 1);
        move_one_char(cur, -1);
        cur->line_size--;
        if (!cur->line_size)
            command_line->idx = -1;
        }
    else if (*buff == 27 && !ft_strncmp(buff + 1, "[1;5C", 5))
        {
        while (line[cur->idx] && line[cur->idx] && !ft_isalnum(line[cur->idx]))
            move_one_char(cur, 1);
        while (ft_isalnum(line[cur->idx]))
            move_one_char(cur, 1);
        idx += 5;
        }
    idx += term_copy(line, buff, cur, command_line);
    update_cursor_pos(cur, line);
    return (idx);
    }
