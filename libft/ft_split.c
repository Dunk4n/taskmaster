/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/08 09:15:31 by niduches          #+#    #+#             */
/*   Updated: 2019/10/16 13:21:32 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

static int	get_nb(char const *s, char c)
{
	int	nb;
	int	i;

	nb = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c && (i == 0 || (s[i - 1] == c)))
			nb++;
		i++;
	}
	return (nb);
}

static char	*fill_in(char **line, char const *s, int *i, char c)
{
	int	j;

	j = 0;
	while (c && s[*i] == c)
		(*i)++;
	j = 0;
	while (s[*i + j] && s[*i + j] != c)
		j++;
	if (!(*line = malloc((j + 1) * sizeof(char))))
		return (NULL);
	j = 0;
	while (s[*i + j] && s[*i + j] != c)
	{
		(*line)[j] = s[*i + j];
		j++;
	}
	(*line)[j] = '\0';
	*i += j;
	return (*line);
}

char		**ft_split(char const *s, char c)
{
	int		i;
	int		word;
	int		nb;
	char	**array;

	if (!s)
		return (NULL);
	nb = get_nb(s, c);
	if (!(array = malloc((nb + 1) * sizeof(char*))))
		return (NULL);
	i = 0;
	word = 0;
	while (word < nb)
	{
		if (!(array[word] = fill_in(array + word, s, &i, c)))
		{
			while (--word >= 0)
				free(array[word]);
			free(array);
			return (NULL);
		}
		word++;
	}
	array[word] = NULL;
	return (array);
}
