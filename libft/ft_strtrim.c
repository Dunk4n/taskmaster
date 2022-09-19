/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/08 09:00:32 by niduches          #+#    #+#             */
/*   Updated: 2019/10/16 19:18:39 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

static int	is_set(char c, char const *set)
{
	size_t	i;

	if (!set)
		return (0);
	i = 0;
	while (set[i])
	{
		if (c == set[i])
			return (1);
		i++;
	}
	return (0);
}

char		*ft_strtrim(char const *s1, char const *set)
{
	int		i;
	int		j;
	int		k;
	char	*str;

	if (!s1)
		return (NULL);
	i = 0;
	while (s1[i] && is_set(s1[i], set))
		i++;
	j = 0;
	while (s1[j])
		j++;
	while (j >= i && j - 1 >= 0 && is_set(s1[j - 1], set))
		j--;
	if (!(str = malloc(((j > 0) ? j - i + 1 : 1) * sizeof(char))))
		return (NULL);
	k = 0;
	while (s1[i + k] && i + k < j)
	{
		str[k] = s1[i + k];
		k++;
	}
	str[k] = '\0';
	return (str);
}
