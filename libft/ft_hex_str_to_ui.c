/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_hex_str_to_ui.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/07/04 11:39:11 by niduches          #+#    #+#             */
/*   Updated: 2019/10/08 17:48:52 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

unsigned int ft_hex_str_to_ui(const char *str)
{
    unsigned int i;
    long         nb;

    i = 0;
    nb = 0;
    while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
        i++;
    while ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F'))
        {
        if (str[i] >= '0' && str[i] <= '9')
            nb = (unsigned int)(nb * 16 + str[i] - '0');
        else if (str[i] >= 'a' && str[i] <= 'f')
            nb = (unsigned int)(nb * 16 + (str[i] - 'a') + 10);
        else
            nb = (unsigned int)(nb * 16 + (str[i] - 'A') + 10);
        i++;
        }
    return (nb);
}
