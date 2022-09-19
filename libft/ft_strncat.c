/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/01 22:13:55 by niduches          #+#    #+#             */
/*   Updated: 2020/06/11 01:53:13 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

char	*ft_strncat(char *dest, char const *src, int nb)
{
	int	len;
	int	i;

	i = 0;
	len = 0;
	while (dest[len])
		len++;
	while (i < nb && src[i] != '\0')
	{
		dest[len + i] = src[i];
		++i;
	}
	dest[len + i] = '\0';
	return (dest);
}
