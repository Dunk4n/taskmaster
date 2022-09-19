/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_float.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 12:04:30 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 14:41:45 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <float.h>
#include "printf.h"

static size_t	put_double(int output, double nb, int len, int *flags)
{
	size_t	i;
	int		j;
	char	c;

	i = put_long_nbr(output, (long long)nb, (!len) ? 1 : len, flags[6]);
	if (flags[2] || flags[7])
		i += write(output, ".", 1);
	else
		return (i);
	nb = nb - (long long)nb;
	if (flags[2] == -1)
		flags[2] = 6;
	j = 0;
	if (nb < 0)
		nb *= -1;
	while (j < flags[2])
	{
		nb *= 10;
		c = (char)(nb) + '0';
		i += write(output, &c, 1);
		nb = nb - (long long)(nb);
		j++;
	}
	return (i);
}

static size_t	put_nb(int output, double nb, int *flags)
{
	size_t	i;

	i = 0;
	if (nb < 0)
		i += write(output, "-", 1);
	if (nb >= 0 && flags[9])
		i += write(output, "+", 1);
	if (nb >= 0 && flags[8] && !flags[9])
		i += write(output, " ", 1);
	if (flags[1])
	{
		i +=
put_double(output, nb, flags[10] - i - ((flags[2] == -1) ? 7 : (flags[2] + 1)), flags);
		return (i);
	}
	return (put_double(output, nb, 0, flags) + i);
}

static size_t	get_len( double nb, int *flags)
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
	while ((long long)(nb / 10))
	{
		nb /= 10;
		len++;
	}
	if (flags[2] == -1)
		add += 7;
	else
		add += flags[2] + ((flags[2] == 0 && !flags[7]) ? 0 : 1);
	return (len + add);
}

int				conv_float_print(int output, double nb, int *flags, int round)
{
	size_t	i;
	size_t	len;

	if (flags[0])
		flags[1] = 0;
	nb = float_round(nb, flags, round);
	i = 0;
	len = get_len(nb, flags);
	if (flags[0])
		i += put_nb(output, nb, flags);
	while (!flags[1] && (int)(i++) < (int)(flags[10] - len))
		write(output, " ", 1);
	if (!flags[0])
		i += put_nb(output, nb, flags);
	return (i + ((!flags[1]) ? -1 : 0));
}
