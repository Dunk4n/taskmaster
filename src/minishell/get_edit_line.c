#include <unistd.h>
#include <stdio.h>
#include "minishell.h"

static void	display_all_command_line(t_cursor *cur, char *line, char move)
{
	int	i;
	int	j;
	int	tmp;

	move_cursor(cur->startx, cur->starty);
	cursor_exec("cd");
	write(1, "$> ", 3);
	j = 0;
	i = 0;
	while (i < cur->line_max)
	{
		tmp = write(1, line, ft_strlen(line));
		line += tmp + 1;
		j += 1 + (tmp + ((!cur->line) ? cur->startx : 0)) / cur->term_col;
		move_cursor(0, cur->starty + j);
		i++;
	}
	if (move)
		move_cursor(cur->x, cur->y);
}

static char	*get_good_line(t_cursor *cur, char *line)
{
	char	*new;
	int		i;

	if (!(new = malloc((cur->line_size + 1) * sizeof(char))))
		return (NULL);
	new[cur->line_size] = '\0';
	i = 0;
	while (i < cur->line_size)
	{
		if (line[i])
			new[i] = line[i];
		else
			new[i] = '\n';
		i++;
	}
	return (new);
}

static int	parse_command(char *line, char *buff, t_cursor *cur, char **new_line)
{
	int		i;
	char	*tmp;

	*new_line = NULL;
	tmp = buff;
	while (*buff)
	{
		i = 0;
		if (!is_term_command(buff, cur, line))
			add_char_in_line(line, buff++, cur);
		else if (!(i = make_term_command(line, buff, cur, cur->command_line)))
		{
			if (*buff != 4)
			{
				display_all_command_line(cur, line, 0);
				if (cur->y == cur->term_line - 1)
					write(1, "\n", 1);
				*new_line = get_good_line(cur, line);
			}
			return (0);
		}
		buff += i;
	}
	*tmp = '\0';
	return (1);
}

int			get_edit_line(t_command_line *command_line, char **new_line)
{
	t_cursor	cur;
	char		line[LINE_SIZE];
	char		buff[128];
	ssize_t		size;

	init_cursor(line, &cur, command_line);
	if (cur.startx < 0 || cur.starty < 0)
		return (0);
	while (1)
	{
		display_all_command_line(&cur, line, 1);
		if ((size = read(STDIN_FILENO, buff, 127)) <= 0)
			return (0);
		buff[size] = '\0';
		if (size > 0 && !parse_command(line, buff, &cur, new_line))
			return (1);
	}
	return (0);
}
