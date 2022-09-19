/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_long_nbr.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/22 12:32:55 by niduches          #+#    #+#             */
/*   Updated: 2019/10/25 19:54:07 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include "printf.h"

size_t	put_long_nbr(int output, long long nb, int len, int nb_c)
{
	size_t	n;
	char	c;

	n = 0;
	if (nb > 9 || nb < -9 || len - 1 > 0)
	{
		n += put_long_nbr(output, (nb < 0) ? (-(nb / 10)) : (nb / 10), len - 1,
(nb_c >= 0) ? nb_c + 1 : -1);
		c = ((nb < 0) ? (-(nb % 10)) : (nb % 10)) + '0';
		n += write(output, &c, 1);
		if (nb > 0 && nb_c > 1 && (nb_c) % 3 == 0)
			n += write(output, ",", 1);
		return (n);
	}
	c = ((nb < 0) ? (-nb) : nb) + '0';
	n += write(output, &c, 1);
	if (nb > 0 && nb_c > 1 && (nb_c) % 3 == 0)
		n += write(output, ",", 1);
	return (n);
}
