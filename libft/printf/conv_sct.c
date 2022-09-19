/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_sct.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/26 12:04:16 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 13:12:34 by niduches         ###   ########.fr       */
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

	i = put_long_nbr(output, (long long)nb, (len < 0) ? 1 : len, flags[6]);
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

static size_t	put_nb_sct(int output, double nb, int *flags, int exp, int len)
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
		i += put_double(output, nb, flags[10] - len + 1, flags);
		i += write(output, (exp < 0) ? "e-" : "e+", 2);
		if (exp < 0)
			exp *= -1;
		i += put_long_nbr(output, exp, 2, -1);
		return (i);
	}
	i += put_double(output, nb, 0, flags);
	i += write(output, (exp < 0) ? "e-" : "e+", 2);
	if (exp < 0)
		exp *= -1;
	i += put_long_nbr(output, exp, 2, -1);
	return (i);
}

static size_t	get_len_sct(double nb, int *flags, int exp)
{
	int	len;
	int add;

	if (flags[0])
		return (0);
	len = 1;
	add = 2;
	while (exp /= 10)
		add++;
	if (add < 4)
		add = 4;
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
		add += flags[2] + ((flags[2] == 0) ? 0 : 1);
	return (len + add);
}

static double	sct_round(double nb, int *flags, int *exp, int round)
{
	double	dec;
	size_t	i;
	size_t	len;

	while (nb != 0 && ((long long)nb >= 10 || (long long)nb <= -10))
	{
		(*exp)++;
		nb /= 10;
	}
	while (nb != 0 && (long long)nb < 1 && (long long)nb > -1)
	{
		(*exp)--;
		nb *= 10;
	}
	if (!round)
		return (nb);
	dec = (nb < 0) ? -0.5 : 0.5;
	i = (flags[2] == -1) ? 6 : flags[2];
	len = 0;
	while (len++ < i)
		dec /= 10;
	nb += dec;
	return (nb);
}

int				conv_sct_print(int output, double nb, int *flags, int round)
{
	size_t	i;
	size_t	len;
	int		exp;

	exp = 0;
	if (flags[0])
		flags[1] = 0;
	nb = sct_round(nb, flags, &exp, round);
	i = 0;
	len = get_len_sct(nb, flags, exp * 10);
	if (flags[0])
		i += put_nb_sct(output, nb, flags, exp, len);
	while (!flags[1] && (int)(i++) < (int)(flags[10] - len))
		write(output, " ", 1);
	if (!flags[0])
		i += put_nb_sct(output, nb, flags, exp, len);
	return (i + ((!flags[1]) ? -1 : 0));
}
