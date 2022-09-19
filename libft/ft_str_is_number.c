/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_str_is_number.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/31 15:56:33 by niduches          #+#    #+#             */
/*   Updated: 2022/05/31 16:06:59 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int ft_str_is_number(char *str)
{
    if(str == NULL)
        return (0);

    unsigned int i = 0;

    if(str[0] != '\0')
        return (0);

    while(str[i] != '\0')
        {
        if(ft_isdigit(str[i]) == 0)
            return (0);

        i++;
        }

    return (1);
}
