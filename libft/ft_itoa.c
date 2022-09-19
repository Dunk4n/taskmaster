/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 10:06:44 by niduches          #+#    #+#             */
/*   Updated: 2019/10/08 14:28:19 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

char	*ft_itoa(int n)
{
	size_t	i;
	long	tmp;
	char	*str;

	i = 1;
	tmp = n;
	if (n < 0)
		i++;
	while (tmp /= 10)
		i++;
	if (!(str = malloc((i + 1) * sizeof(char))))
		return (NULL);
	str[i--] = '\0';
	if (n < 0)
		str[0] = '-';
	if (n == 0)
		str[0] = '0';
	while (n)
	{
		str[i--] = ((n < 0) ? -1 : 1) * (n % 10) + '0';
		n /= 10;
	}
	return (str);
}
