/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/03 22:41:23 by niduches          #+#    #+#             */
/*   Updated: 2019/10/16 15:40:14 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

size_t	ft_strlcat(char *dst, const char *src, size_t dstsize)
{
	size_t	i;
	size_t	len;
	size_t	j;

	i = 0;
	while (dst[i])
		i++;
	len = i;
	j = 0;
	while (dstsize > 0 && i < dstsize - 1 && src[j])
		dst[i++] = src[j++];
	if (dstsize > 0)
		dst[i] = '\0';
	i = 0;
	while (src[i])
		i++;
	if (dstsize < len)
		return (i + dstsize);
	return (i + len);
}
