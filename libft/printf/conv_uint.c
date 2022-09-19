/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_uint.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/14 19:38:02 by niduches          #+#    #+#             */
/*   Updated: 2019/10/26 13:22:16 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "printf.h"

static unsigned long long	get_nb(va_list list, int *flags)
{
	if (flags[4] == 1)
		return (va_arg(list, unsigned long));
	if (flags[4] == 2)
		return (va_arg(list, unsigned long long));
	if (flags[5] == 1)
		return ((unsigned short)va_arg(list, unsigned int));
	if (flags[5] == 2)
		return ((unsigned char)va_arg(list, unsigned int));
	return (va_arg(list, unsigned int));
}

static size_t				put_ulong_nbr(int output, unsigned long long nb, int len,
int nb_c)
{
	size_t	n;
	char	c;

	n = 0;
	if (nb > 9 || len - 1 > 0)
	{
		n += put_ulong_nbr(output, nb / 10, len - 1, (nb_c >= 0) ? nb_c + 1 : -1);
		c = (nb % 10) + '0';
		n += write(output, &c, 1);
		if (nb > 0 && nb_c > 1 && (nb_c) % 3 == 0)
			n += write(output, ",", 1);
		return (n);
	}
	c = nb + '0';
	n += write(output, &c, 1);
	if (nb > 0 && nb_c > 1 && (nb_c) % 3 == 0)
		n += write(output, ",", 1);
	return (n);
}

static size_t				put_nb(int output, unsigned long long nb, int *flags)
{
	size_t	i;

	i = 0;
	if (flags[1])
		return (put_ulong_nbr(output, nb, flags[10] - i, flags[6]) + i);
	else
		return (put_ulong_nbr(output, nb, flags[2], flags[6]) + i);
}

static size_t				get_len(unsigned long long nb, int *flags)
{
	size_t	len;

	if (flags[0] || (!flags[2] && !nb))
		return (0);
	len = 1;
	while (nb /= 10)
		len++;
	if (flags[2] != -1)
		return ((len < (size_t)flags[2]) ? (size_t)flags[2] : len);
	return (len);
}

int							conv_uint(int output, va_list list, int *flags)
{
	unsigned long long	nb;
	size_t				i;
	size_t				len;

	if (flags[2] >= 0 || flags[0])
		flags[1] = 0;
	nb = get_nb(list, flags);
	i = 0;
	len = get_len(nb, flags);
	if (flags[0] && (flags[2] || nb))
		i += put_nb(output, nb, flags);
	while (!flags[1] && (int)i < (int)(flags[10] - len))
	{
		write(output, " ", 1);
		i++;
	}
	if (!flags[0] && (flags[2] || nb))
		i += put_nb(output, nb, flags);
	return (i);
}
