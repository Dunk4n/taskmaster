/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoui.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/04 11:39:11 by niduches          #+#    #+#             */
/*   Updated: 2019/10/08 17:48:52 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

unsigned int ft_atoui(const char *str)
{
    unsigned int i;
    long         nb;

    i = 0;
    nb = 0;
    while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
        i++;
    while (str[i] >= '0' && str[i] <= '9')
        nb = (unsigned int)(nb * 10 + str[i++] - '0');
    return (nb);
}
