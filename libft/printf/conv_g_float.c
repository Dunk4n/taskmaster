/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_g_float.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/27 23:32:41 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 14:42:36 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <float.h>
#include "printf.h"

static int	make_g_decimal(int output, double nb, int *flags, size_t len)
{
	long long	i;
	long long	tmp;
	double		tmp_d;

	tmp_d = nb - (long long)nb;
	tmp = 0;
	i = 0;
	while (i < flags[2] - (int)len)
	{
		tmp_d *= 10;
		i++;
		if ((int)tmp_d != 0)
			tmp = i;
		tmp_d = tmp_d - (int)tmp_d;
	}
	if (!flags[7])
		flags[2] = tmp;
	else
		flags[2] -= len;
	return (conv_float_print(output, nb, flags, 0));
}

int			make_g(int output, double nb, int *flags)
{
	size_t		len;
	long long	tmp;

	tmp = nb;
	len = 0;
	while (tmp)
	{
		tmp /= 10;
		len++;
	}
	if ((int)len >= flags[2] || (nb - (long long)nb) == 0)
	{
		flags[2] = 0;
		return (conv_float_print(output, nb, flags, 0));
	}
	return (make_g_decimal(output, nb, flags, len));
}
