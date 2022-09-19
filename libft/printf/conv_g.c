/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_g.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/27 18:13:07 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 14:31:49 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <float.h>
#include "printf.h"

static int		if_sct_up(double nb, int *flags)
{
	int			len;
	long long	tmp;

	tmp = nb;
	len = 0;
	while (tmp)
	{
		tmp /= 10;
		len++;
	}
	if (len > flags[2] && len > 1)
		return (1);
	return (0);
}

static int		if_sct_down(double nb)
{
	int			len;
	double		tmp;
	long long	i;

	if ((long long)nb != 0 || nb == 0)
		return (0);
	tmp = nb - (long long)nb;
	len = -1;
	i = 0;
	while (i < 6)
	{
		tmp *= 10;
		if (len == -1 && (int)tmp != 0)
			len = i;
		i++;
		tmp = tmp - (int)tmp;
	}
	if (len >= 4 || len < 0)
		return (1);
	return (0);
}

static int		make_sct_g_up(int output, double nb, int *flags)
{
	int			i;
	int			len;
	double		tmp;

	tmp = nb;
	i = 0;
	len = 0;
	while ((long long)tmp / 10)
		tmp /= 10;
	tmp = tmp - (int)tmp;
	while (i < flags[2] - 1)
	{
		tmp *= 10;
		i++;
		if ((int)tmp != 0)
			len = i;
		tmp = tmp - ((int)tmp);
	}
	flags[2] = len;
	return (conv_sct_print(output, nb, flags, 1));
}

static int		make_sct_g_down(int output, double nb, int *flags)
{
	int			i;
	int			len;
	double		tmp;

	tmp = nb;
	i = 0;
	len = 0;
	while (!(int)tmp)
		tmp *= 10;
	tmp = tmp - (int)tmp;
	while (i < flags[2] - 1)
	{
		tmp *= 10;
		i++;
		if ((int)tmp != 0)
			len = i;
		tmp = tmp - ((int)tmp);
	}
	flags[2] = len;
	return (conv_sct_print(output, nb, flags, 1));
}

int				conv_g(int output, va_list list, int *flags)
{
	double		nb;

	if (flags[2] == -1)
		flags[2] = 6;
	if (flags[0])
		flags[1] = 0;
	nb = va_arg(list, double);
	if (if_sct_up(nb, flags))
		return (make_sct_g_up(output, nb, flags));
	if (if_sct_down(nb))
		return (make_sct_g_down(output, nb, flags));
	return (make_g(output, float_round(nb, flags, 1), flags));
}
