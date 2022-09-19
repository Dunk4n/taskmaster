/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/10 15:05:40 by niduches          #+#    #+#             */
/*   Updated: 2021/05/15 11:24:38 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int		ft_strcmp(char const *s1, char const *s2)
{
	unsigned char	*tmp1;
	unsigned char	*tmp2;

	tmp1 = (unsigned char*)s1;
	tmp2 = (unsigned char*)s2;
	while (*tmp1 != '\0' || *tmp2 != '\0')
	{
		if (*tmp1 != *tmp2)
			return (((int)*tmp1) - ((int)*tmp2));
		++tmp1;
		++tmp2;
	}
	return (0);
}
