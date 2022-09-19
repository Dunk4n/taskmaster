/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/12 12:29:40 by niduches          #+#    #+#             */
/*   Updated: 2020/06/24 00:53:52 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "printf.h"

static size_t	pass_flags(const char *str, char *flag)
{
	size_t	i;

	*flag = 0;
	if (!*str || *str != '%')
		return (0);
	i = 1;
	while (ft_isflag(str[i]) || (str[i] > '0' && str[i] <= '9'))
		i++;
	if (!ft_isconv(str[i]))
		return (i);
	*flag = 1;
	return (i + 1);
}

static void		init_printf(size_t *nb_print, int *to_print, int *i)
{
	*nb_print = 0;
	*to_print = 0;
	*i = 0;
}

int				ft_vfprintf(int output, const char *str, va_list list)
{
	int		i;
	int		to_print;
	size_t	nb_print;
	char	flag;

	if (!str)
		return (0);
	init_printf(&nb_print, &to_print, &i);
	while (str[i] && to_print != -1)
	{
		to_print = 0;
		while (str[i + to_print] && str[i + to_print] != '%')
			to_print++;
		nb_print += ft_putnstr(output, str + i, to_print);
		i += to_print;
		to_print = pass_flags(str + i, &flag);
		if (flag)
			nb_print += ft_makeconv(output, str + i, &to_print, list, nb_print);
		i += to_print;
	}
	return ((to_print != -1) ? (int)nb_print : -1);
}

int				ft_fprintf(int output, const char *str, ...)
{
	va_list	list;
	int		ret;

	if (!str)
		return (0);
	va_start(list, str);
    ret = ft_vfprintf(output, str, list);
	va_end(list);
	return (ret);
}

int				ft_printf(const char *str, ...)
{
	va_list	list;
	int		ret;

	if (!str)
		return (0);
	va_start(list, str);
    ret = ft_vfprintf(STDOUT_FILENO, str, list);
	va_end(list);
	return (ret);
}
