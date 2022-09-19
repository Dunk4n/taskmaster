/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 18:35:56 by niduches          #+#    #+#             */
/*   Updated: 2019/10/10 17:08:57 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*str;
	size_t	i;
	size_t	size;

	if (!s)
		return (NULL);
	size = 0;
	while (s[size])
		size++;
	i = 0;
	while (size >= start + 1 && s[start + i] && i < len)
		i++;
	if (!(str = malloc((i + 1) * sizeof(char))))
		return (NULL);
	i = 0;
	while (size >= start + 1 && s[start + i] && i < len)
	{
		str[i] = s[start + i];
		i++;
	}
	str[i] = '\0';
	return (str);
}
