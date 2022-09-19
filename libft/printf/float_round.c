/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   float_round.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/28 14:27:39 by niduches          #+#    #+#             */
/*   Updated: 2019/10/28 14:29:04 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <float.h>
#include "printf.h"

double	float_round(double nb, int *flags, int round)
{
	double	dec;
	size_t	i;
	size_t	len;

	if (!round)
		return (nb);
	dec = (nb < 0) ? -0.5 : 0.5;
	len = 0;
	i = (flags[2] == -1) ? 6 : flags[2];
	while (len++ < i)
		dec /= 10;
	nb += dec;
	return (nb);
}
