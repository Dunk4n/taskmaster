/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_uhex.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/14 19:41:03 by niduches          #+#    #+#             */
/*   Updated: 2019/10/22 12:38:13 by niduches         ###   ########.fr       */
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

static size_t				put_long_nbr_hex(int output, unsigned long long nb, int len)
{
	size_t	n;
	char	c;

	n = 0;
	if (nb > 15 || len - 1 > 0)
	{
		n += put_long_nbr_hex(output, nb / 16, len - 1);
		c = UHEX[nb % 16];
		n += write(output, &c, 1);
		return (n);
	}
	c = UHEX[nb];
	n += write(output, &c, 1);
	return (n);
}

static size_t				put_nb(int output, unsigned long long nb, int *flags)
{
	int	tmp;

	tmp = ((flags[7] && nb) ? 2 : 0);
	if (flags[7] && nb)
		write(output, "0X", 2);
	if (flags[1])
		return (put_long_nbr_hex(output, nb, flags[10] - tmp) + tmp);
	else
		return (put_long_nbr_hex(output, nb, flags[2]) + tmp);
}

static size_t				get_len(unsigned long long nb, int *flags)
{
	int	len;

	if (flags[0] || (!nb && !flags[2]))
		return (0);
	len = 1;
	if (flags[7] && nb)
		len += 2;
	while (nb /= 16)
		len++;
	if (flags[2] != -1)
		return ((len < flags[2]) ? flags[2] : len);
	return (len);
}

int							conv_uhex(int output, va_list list, int *flags)
{
	unsigned long long	nb;
	size_t				i;
	size_t				len;

	if (flags[2] >= 0 || flags[0])
		flags[1] = 0;
	nb = get_nb(list, flags);
	i = 0;
	len = get_len(nb, flags);
	if (flags[0] && (nb || flags[2]))
		i += put_nb(output, nb, flags);
	while (!flags[1] && (int)i < (int)(flags[10] - len))
	{
		write(output, " ", 1);
		i++;
	}
	if (!flags[0] && (nb || flags[2]))
		i += put_nb(output, nb, flags);
	return (i);
}
