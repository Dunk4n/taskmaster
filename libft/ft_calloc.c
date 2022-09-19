/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 17:43:57 by niduches          #+#    #+#             */
/*   Updated: 2019/10/09 09:46:29 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

void	*ft_calloc(size_t count, size_t size)
{
	char	*ptr;
	size_t	i;
	size_t	len;

	len = count * size;
	ptr = malloc(len);
	if (!ptr)
		return (NULL);
	i = 0;
	while (i < len)
		ptr[i++] = 0;
	return (ptr);
}
