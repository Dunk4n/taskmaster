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

int		ft_strcmp_no_case(char const *s1, char const *s2)
{
	unsigned char	*tmp1;
	unsigned char	*tmp2;
	unsigned char	tmp1_char;
	unsigned char	tmp2_char;

    if ((s1 == 0) || (s2 == 0))
    {
        return (1);
    }
	tmp1 = (unsigned char*)s1;
	tmp2 = (unsigned char*)s2;
	tmp1_char = '\0';
	tmp2_char = '\0';
	while (*tmp1 != '\0' || *tmp2 != '\0')
	{
		tmp1_char = *tmp1;
		tmp2_char = *tmp2;
		if(tmp1_char >= 'A' && tmp1_char <= 'Z')
			tmp1_char = tmp1_char + 32;
		if(tmp2_char >= 'A' && tmp2_char <= 'Z')
			tmp2_char = tmp2_char + 32;
		if (tmp1_char != tmp2_char)
			return (((int) tmp1_char) - ((int) tmp2_char));
		++tmp1;
		++tmp2;
	}
	return (0);
}
