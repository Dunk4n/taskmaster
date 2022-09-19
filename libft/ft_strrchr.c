/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 16:37:49 by niduches          #+#    #+#             */
/*   Updated: 2019/10/08 15:46:42 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

char	*ft_strrchr(const char *s, int c)
{
	size_t	i;
	size_t	tmp;

	i = 0;
	tmp = 0;
	while (s[i] != '\0')
	{
		if (s[i] == (char)c)
			tmp = i;
		i++;
	}
	if ((char)c == '\0')
		return ((void*)s + i);
	if (s[tmp] == (char)c)
		return ((void*)s + tmp);
	return (NULL);
}
