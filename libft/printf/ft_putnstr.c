/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/01 22:17:07 by niduches          #+#    #+#             */
/*   Updated: 2019/12/01 22:17:52 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>

size_t	ft_putnstr(int output, const char *str, size_t n)
{
	size_t	i;

	if (!str || !n)
		return (0);
	i = 0;
	while (str[i] && i < n)
		i++;
	write(output, str, i);
	return (i);
}
