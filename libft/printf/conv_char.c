/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_char.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/13 17:32:16 by niduches          #+#    #+#             */
/*   Updated: 2019/10/19 16:11:04 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include "printf.h"

static long long	get_char(va_list list, int *flags)
{
	wint_t	c;

	if (flags[4] == 1)
	{
		c = (wint_t)va_arg(list, wint_t);
		return (c);
	}
	if (flags[4] == 2)
		return (va_arg(list, unsigned long long));
	if (flags[5] == 1)
		return ((unsigned short)va_arg(list, int));
	if (flags[5] == 2)
		return ((unsigned char)va_arg(list, int));
	return ((unsigned char)va_arg(list, int));
}

int					conv_char(int output, va_list list, int *flags)
{
	long long		c;
	char			padding;
	size_t			i;

	c = get_char(list, flags);
	if (flags[4] == 1 && (c < 0 || c > 255))
		return (-1);
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
