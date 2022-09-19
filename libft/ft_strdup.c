/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 17:50:38 by niduches          #+#    #+#             */
/*   Updated: 2019/11/27 17:14:49 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

char	*ft_strdup(const char *s1)
{
	char	*str;
	size_t	i;

	i = 0;
	while (s1[i])
		i++;
	if (!(str = malloc((i + 1) * sizeof(char))))
		return (NULL);
	str[i] = '\0';
	while (i != 0)
	{
		i--;
		str[i] = s1[i];
	}
	return (str);
}
