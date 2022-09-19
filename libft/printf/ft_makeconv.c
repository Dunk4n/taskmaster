/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putconv.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/12 16:52:12 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 13:06:15 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "printf.h"

static int		conv_percent(int output, va_list list, int *flags)
{
	long long		c;
	char			padding;
	size_t			i;

	(void)list;
	c = '%';
	padding = (!flags[0] && flags[1]) ? '0' : ' ';
	if (flags[0])
		write(output, &c, 1);
	i = 0;
	while (flags[10] && (int)i < flags[10] - 1)
	{
		write(output, &padding, 1);
		i++;
	}
	if (!flags[0])
		write(output, &c, 1);
	return (i + 1);
}

static int		conv_float(int output, va_list list, int *flags)
{
	return (conv_float_print(output, va_arg(list, double), flags, 1));
}

static int		conv_sct(int output, va_list list, int *flags)
{
	return (conv_sct_print(output, va_arg(list, double), flags, 1));
}

int				(*g_f[13])(int, va_list, int*) = {conv_char, conv_str, conv_ptr,
	conv_int, conv_int, conv_uint, conv_hex, conv_uhex, conv_percent,
	conv_nb_print_char, conv_float, conv_g, conv_sct};

int				ft_makeconv(int output, const char *str, int *size,
va_list list, size_t nb_print)
{
	size_t	i;
	int		res;
	int		flags[NB_FLAGS + 1];

	i = 0;
	while (i <= NB_FLAGS)
		flags[i++] = 0;
	flags[6] = -1;
	flags[2] = -1;
	get_flags(flags, str + 1, *size - 2, list);
	i = 0;
	res = 0;
	while (CONVERSIONS[i])
		if (CONVERSIONS[i++] == str[*size - 1])
		{
			if (CONVERSIONS[i - 1] == 'n')
				flags[0] = (int)nb_print;
			res = (g_f[i - 1])(output, list, flags);
			break ;
		}
	if (res == -1)
		*size = -1;
	return (res);
}
