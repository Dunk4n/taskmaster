/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_nbr_print_char.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/21 13:11:16 by niduches          #+#    #+#             */
/*   Updated: 2019/10/21 13:35:50 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include "printf.h"

int		conv_nb_print_char(int output, va_list list, int *flags)
{
	void	*ptr;

    (void) output;
	ptr = va_arg(list, int*);
	if (flags[4] == 1)
		*((long*)ptr) = flags[0];
	else if (flags[4] == 2)
		*((long long*)ptr) = flags[0];
	else if (flags[5] == 1)
		*((short*)ptr) = flags[0];
	else if (flags[5] == 2)
		*((char*)ptr) = flags[0];
	else
		*((int*)ptr) = flags[0];
	return (0);
}
