/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_int.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/14 12:21:33 by niduches          #+#    #+#             */
/*   Updated: 2019/10/25 19:44:31 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include "printf.h"

static long long	get_nb(va_list list, int *flags)
{
	if (flags[4] == 1)
		return (va_arg(list, long));
	if (flags[4] == 2)
		return (va_arg(list, long long));
	if (flags[5] == 1)
		return ((short)va_arg(list, int));
	if (flags[5] == 2)
		return ((char)va_arg(list, int));
	return (va_arg(list, int));
}

static size_t		put_nb(int output, long long nb, int *flags)
{
	size_t	i;

	i = 0;
	if (nb < 0)
		i += write(output, "-", 1);
	if (nb >= 0 && flags[9])
		i += write(output, "+", 1);
	if (nb >= 0 && flags[8] && !flags[9])
		i += write(output, " ", 1);
	if (!flags[2] && !nb)
		return (i);
	if (flags[1])
		return (put_long_nbr(output, nb, flags[10] - i, flags[6]) + i);
	else
		return (put_long_nbr(output, nb, flags[2], flags[6]) + i);
}

static size_t		get_len(long long nb, int *flags)
{
	int	len;
	int add;

	if (flags[0])
		return (0);
	len = 1;
	add = 0;
	if (nb < 0 || flags[8] || flags[9])
		add++;
	if (!flags[2] && !nb)
		return (add);
	while (nb /= 10)
		len++;
	if (flags[2] != -1)
		return ((len < flags[2]) ? flags[2] + add : len + add);
	return (len + add);
}

int					conv_int(int output, va_list list, int *flags)
{
	long long	nb;
	size_t		i;
	size_t		len;

	if (flags[2] >= 0 || flags[0])
		flags[1] = 0;
	nb = get_nb(list, flags);
	i = 0;
	len = get_len(nb, flags);
	if (flags[0])
		i += put_nb(output, nb, flags);
	while (!flags[1] && (int)i < (int)(flags[10] - len))
	{
		write(output, " ", 1);
		i++;
	}
	if (!flags[0])
		i += put_nb(output, nb, flags);
	return (i);
}
