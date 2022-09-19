/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isconv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/12 14:03:10 by niduches          #+#    #+#             */
/*   Updated: 2019/10/21 13:09:14 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "printf.h"

int		ft_isconv(char c)
{
	size_t	i;

	i = 0;
	while (CONVERSIONS[i])
	{
		if (c == CONVERSIONS[i])
			return (1);
		i++;
	}
	return (0);
}
